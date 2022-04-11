#include "fileview.h"
#include "fileviewheader.h"
#include "filemodel.h"
#include "filefiltermodel.h"
#include "lineeditvaluedelegate.h"
#include "filerenamedialog.h"
#include "fileeditdialog.h"
#include "settings.h"
#include "directoryremover.h"
#include "filemover.h"
#include "filecopier.h"
#include "partcache.h"

#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QMenu>
#include <QHeaderView>
#include <QShortcut>
#include <QtDebug>


FileView::FileView(QWidget *parent) :
	QTreeView(parent)
{
	m_model = new FileModel(this);
	m_proxy = new FileFilterModel(this);

	m_proxy->setSourceModel(m_model);
	setModel(m_proxy);

    setSortingEnabled(true);
    sortByColumn(100, Qt::AscendingOrder);

	m_header = new FileViewHeader(m_model, this);
	setHeader(m_header);

	connect(m_header, SIGNAL(filterColumn(int,QString)),
			m_proxy, SLOT(filterColumn(int,QString)));

	setItemDelegate(new LineEditValueDelegate(this));
	setEditTriggers(QAbstractItemView::SelectedClicked);

	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_model, SIGNAL(directoryLoaded(QString)),
	        this, SLOT(resizeColumnToContents()));
	connect(PartCache::get(), SIGNAL(directoryRenamed(QString,QString)),
			this, SLOT(directoryRenamed(QString,QString)));

	connect(this, SIGNAL(activated(QModelIndex)),
	        this, SLOT(handleActivated(QModelIndex)));
	connect(this, SIGNAL(clicked(QModelIndex)),
	        this, SLOT(handleActivated(QModelIndex)));
	connect(this, SIGNAL(doubleClicked(QModelIndex)),
			this, SLOT(openPart(QModelIndex)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)),
			this, SLOT(showContextMenu(QPoint)));

	connect(MetadataCache::get(), SIGNAL(cleared()), this, SLOT(reloadParts()));
}

void FileView::setDirectory(const QString &path)
{
	if (path.isEmpty())
	{
		return;
	}
	m_path = path;
	// it has to be reset here because calling QFileSystemModel's reset
	// or begin/end alternatives results in "/" as a root path
	m_model->setDirectory(m_path);
	m_proxy->resetFilters();
	//setRootIndex(m_proxy->mapFromSource(m_model->setRootPath(m_path)));
	resizeColumnToContents();

	sortByColumn(0, MetadataCache::get()->sortOrder(m_path));
}

void FileView::refreshModel()
{
	setDirectory(m_path);
	m_model->refreshModel();
}

void FileView::reloadParts()
{
	setDirectory(m_path);
	m_model->reloadParts();
}

void FileView::resizeColumnToContents()
{
	int columnCnt = m_model->columnCount(QModelIndex());
	for (int i = 0; i < columnCnt; i++)
	{
		QTreeView::resizeColumnToContents(i);
		// hack. Probably some QFontMetrics for header should be used. But not urgent for now.
		if (columnWidth(i) < 50)
			setColumnWidth(i, 50);
	}
}

void FileView::settingsChanged()
{
	m_model->settingsChanged();
	m_proxy->setShowProeVersions(Settings::get()->ShowProeVersions);
	refreshModel();
}

QFileInfo FileView::fileInfo(const QModelIndex &filteredIndex)
{
	QModelIndex ix = m_proxy->mapToSource(filteredIndex);
	return m_model->fileInfo(ix);
}

void FileView::createIndexHtmlFile(const QString &text, const QString &fileBase)
{
	QDir techSpecDir(m_path + "/" + METADATA_DIR);

	if(!techSpecDir.exists())
		techSpecDir.mkdir(techSpecDir.absolutePath());

	QString lang = Settings::get()->getCurrentLanguageCode().left(2);
	QString htmlFile = techSpecDir.absolutePath() + "/" + fileBase + "_" + lang + ".html";
	QFile indexFile(htmlFile);

	if(indexFile.exists())
	{
		if (QMessageBox::warning(0,
		                         tr("HTML index file already exists"),
		                         tr("HTML index for already exists, would you like to overwrite it?").arg(m_path),
		                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
	}

	if(!indexFile.open(QIODevice::WriteOnly))
		return; // FIXME: Notify user on failure?

	QByteArray htmlIndex = QString("<html>\n"
	                               "	<head>\n"
	                               "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
	                               "	</head>\n"
	                               "</html>\n").arg(text).toUtf8();

	indexFile.write(htmlIndex);
	indexFile.close();
}

void FileView::moveParts()
{
	FileMover *mv = new FileMover(this);
	m_model->moveParts(mv);
}

void FileView::deleteParts()
{
	DirectoryRemover *rm = new DirectoryRemover(this);
	rm->setMessage(tr("Please wait while the parts are being removed..."));
	m_model->deleteParts(rm);
}

void FileView::refreshRequested()
{
	m_model->reloadParts();
}

QString FileView::currentPath()
{
	return m_path;
}

void FileView::copyToWorkingDir()
{
	FileCopier *cp = new FileCopier(this);
	cp->setMessage(tr("Please wait while the parts are being copied..."));
	m_model->copyToWorkingDir(cp);
}

void FileView::directoryChanged()
{
	m_model->reloadParts();
	m_header->newDirectory(m_path);
	sortByColumn(0, MetadataCache::get()->sortOrder(m_path));
}

void FileView::scrollContentsBy(int dx, int dy)
{
	QTreeView::scrollContentsBy(dx, dy);

	if (dx != 0)
		m_header->fixComboPositions();
}

QModelIndex FileView::findNextPartIndex(const QModelIndex &from)
{
	QString name = fileInfo(from).baseName();
	QModelIndex index = from;

	while (true) {
		index = indexBelow(index);

		if (!index.isValid())
			return index;

		if (fileInfo(index).baseName() == name)
			continue;

		return index;
	}
}

void FileView::handleActivated(const QModelIndex &index)
{
	// open productview only when user clicks on the thumbnail
	if (index.column() == 1)
		emit previewProductView(fileInfo(index));
	else
		emit hideProductView();
}

void FileView::openPart(const QModelIndex &index)
{
	QFileInfo fi(fileInfo(index));

	if (fi.isDir()) {
		emit openPartDirectory(fi);
		return;
	}

	FileMetadata f(fi);

	switch (f.type)
	{
	case FileType::PRT_PROE:
	case FileType::ASM:
	case FileType::DRW:
	case FileType::FRM:
	case FileType::NEU_PROE:
	{
		QString exe = Settings::get()->ProeExecutable;
		qDebug() << "Starting ProE:" << exe << f.fileInfo.absoluteFilePath() << "; working dir" << m_path;
		bool ret = QProcess::startDetached(
			exe,
			QStringList() << f.fileInfo.absoluteFilePath(),
			m_path
		);

		if (!ret)
		{
			QMessageBox::information(
				this,
				tr("ProE Startup Error"),
				tr("An error occured while ProE has been requested to start"),
				QMessageBox::Ok
			);
		}
		break;
	}
	default:
		QDesktopServices::openUrl(QUrl::fromLocalFile(f.fileInfo.absoluteFilePath()));
	}
}

void FileView::showContextMenu(const QPoint &point)
{
	QModelIndex i = currentIndex();

	if (!i.isValid())
		return;

	QFileInfo fi = fileInfo(i);

	QMenu *menu = new QMenu(this);

	if (fi.isFile()) {
		menu->addAction(QIcon(":/gfx/edit-rename.png"), tr("Rename"),
						this, SLOT(renameFile()));
	}

	menu->addAction(QIcon(":/gfx/document-edit.png"), tr("Edit"),
					this, SLOT(editFile()));
	menu->exec(mapToGlobal(point));
	menu->deleteLater();
}

void FileView::renameFile()
{
	QModelIndex index = currentIndex();

	if (!index.isValid())
		return;

	FileRenameDialog dlg(m_path, fileInfo(index));

	if (dlg.exec() == QDialog::Accepted)
		m_model->reloadParts();
}

void FileView::editFile()
{
	QModelIndex index = currentIndex();

	while (true) {
		if (!index.isValid())
			return;

		// Highlight the edited item
		setCurrentIndex(index);

		FileEditDialog dlg(m_path, fileInfo(index).baseName(), this);

		if (dlg.exec() != QDialog::Accepted)
			return;

		dlg.save();

		if (!dlg.editNext())
			return;

		index = findNextPartIndex(index);
	}
}

void FileView::directoryRenamed(const QString &oldName, const QString &newName)
{
	if (m_path == oldName)
		m_path = newName;
}
