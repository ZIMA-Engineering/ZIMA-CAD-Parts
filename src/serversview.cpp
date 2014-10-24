#include "serversview.h"
#include "serversmodel.h"
#include "zimautils.h"
#include "settings.h"
#include "settingsdialog.h"

#include <QHeaderView>
#include <QDesktopServices>
#include <QSignalMapper>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>


ServersView::ServersView(const QString &rootPath, QWidget *parent) :
	QTreeView(parent),
	m_path(rootPath)
{
	// boss requirement - icons shoudl have be at least 32px sized
	setStyleSheet("icon-size: 32px;");

	m_proxy = new ServersProxyModel(this);

	m_model = new ServersModel(this);
	m_proxy->setSourceModel(m_model);

	setModel(m_proxy);
	refreshModel();

	header()->close();

	setContextMenuPolicy(Qt::CustomContextMenu);

	m_signalMapper = new QSignalMapper(this);

	connect(m_signalMapper, SIGNAL(mapped(QString)),
	        this, SLOT(spawnZimaUtilityOnDir(QString)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(showContextMenu(QPoint)));
	connect(this, SIGNAL(clicked(QModelIndex)),
	        this, SLOT(modelClicked(QModelIndex)));
	connect(MetadataCache::get(), SIGNAL(cleared()),
	        this, SLOT(refreshModel()));
}

void ServersView::refreshModel()
{
	// it has to be reset here because calling QFileSystemModel's reset
	// or begin/end alternatives results in "/" as a root path
	setRootIndex(m_proxy->mapFromSource(m_model->setRootPath(m_path)));
}

void ServersView::modelClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	emit directorySelected(currentFileInfo().absoluteFilePath());
}

QFileInfo ServersView::currentFileInfo()
{
	QModelIndex index = currentIndex();
	if (!index.isValid())
		return QFileInfo();

	QModelIndex srcIndex = m_proxy->mapToSource(index);
	return m_model->fileInfo(srcIndex).absoluteFilePath();
}

void ServersView::showContextMenu(const QPoint &point)
{
	QModelIndex i = currentIndex();

	if (!i.isValid())
		return;

	QMenu *menu = new QMenu(this);

	menu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Open"), this, SLOT(indexOpenPath()));
	menu->addAction(QIcon(":/gfx/gohome.png"), tr("Set as working directory"), this, SLOT(setWorkingDirectory()));

	menu->addSeparator();

	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PTC-Cleaner.png"), tr("Clean with ZIMA-PTC-Cleaner"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaPtcCleaner));
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-CAD-Sync.png"), tr("Sync with ZIMA-CAD-Sync"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaCadSync));
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PS2PDF.png"), tr("Convert postscript to PDF with ZIMA-PS2PDF"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaPs2Pdf));
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-STEP-Edit.png"), tr("Edit step files with ZIMA-STEP-Edit"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaStepEdit));

	menu->exec(mapToGlobal(point));
	menu->deleteLater();
}

void ServersView::indexOpenPath()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(currentFileInfo().absoluteFilePath()));
}

void ServersView::spawnZimaUtilityOnDir(const QString &label)
{
	QString executable = Settings::get()->ExternalPrograms[label];

	if (executable.isEmpty())
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Please first configure path to %1 executable.").arg(label));
		emit showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	if (!QFile::exists(executable))
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Path '%1' to %2 executable does not exists!").arg(executable).arg(label));
		emit showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	QStringList args;
	args << currentFileInfo().absoluteFilePath();
	QProcess::startDetached(executable, args);
}

void ServersView::setWorkingDirectory()
{
	Settings::get()->WorkingDir = currentFileInfo().absoluteFilePath();
	emit workingDirChanged();
}

bool ServersView::navigateToDirectory(const QString &path)
{
	// find the common root path. Then ise the index.
	// note: all QFileSystemModels have the index(path) so we need to
	// handle prefixes. ::match() did not work here.
	QString root = m_model->filePath(m_proxy->mapToSource(rootIndex()));
	if (!path.startsWith(root))
		return false;

	setCurrentIndex(m_proxy->mapFromSource(m_model->index(path)));
	return true;
}
