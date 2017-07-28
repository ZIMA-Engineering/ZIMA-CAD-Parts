#include "fileview.h"
#include "filemodel.h"
#include "filefiltermodel.h"
#include "settings.h"

#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
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
	setEditTriggers(QAbstractItemView::SelectedClicked);

	connect(m_model, SIGNAL(directoryLoaded(QString)),
	        this, SLOT(resizeColumnToContents()));

	connect(this, SIGNAL(activated(QModelIndex)),
	        this, SLOT(handleActivated(QModelIndex)));
	connect(this, SIGNAL(clicked(QModelIndex)),
	        this, SLOT(handleActivated(QModelIndex)));
	connect(this, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(openInProE(QModelIndex)));

	connect(MetadataCache::get(), SIGNAL(cleared()), this, SLOT(refreshModel()));
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
	//setRootIndex(m_proxy->mapFromSource(m_model->setRootPath(m_path)));
	resizeColumnToContents();
}

void FileView::refreshModel()
{
	setDirectory(m_path);
    m_model->refreshModel();
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
	QDir techSpecDir(m_path + "/" + TECHSPEC_DIR);

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

void FileView::deleteParts()
{
	m_model->deleteParts();
}

void FileView::refreshRequested()
{
	MetadataCache::get()->clear(m_path);
}

void FileView::reloadProeMeta()
{
    MetadataCache::get()->metadata(m_path)->reloadProe(m_model->fileInfoList());
    m_model->refreshModel();
}

void FileView::copyToWorkingDir()
{
	m_model->copyToWorkingDir();
}

void FileView::handleActivated(const QModelIndex &index)
{
	// open productview only when user clicks on the thumbnail
	if (index.column() == 1)
		emit previewProductView(fileInfo(index));
	else
		emit hideProductView();
}

void FileView::openInProE(const QModelIndex &index)
{
	QFileInfo fi(fileInfo(index));
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
		bool ret = QProcess::startDetached(exe, QStringList() << f.fileInfo.absoluteFilePath(),
		                                   m_path);
		if (!ret)
			QMessageBox::information(this, tr("ProE Startup Error"),
			                         tr("An error occured while ProE has been requested to start"),
			                         QMessageBox::Ok);
		break;
	}
	default:
		QDesktopServices::openUrl(QUrl::fromLocalFile(f.fileInfo.absoluteFilePath()));
	}
}
