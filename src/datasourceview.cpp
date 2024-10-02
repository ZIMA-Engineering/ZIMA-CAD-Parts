#include "datasourceview.h"
#include "datasourcemodel.h"
#include "zimautils.h"
#include "settings.h"
#include "settingsdialog.h"
#include "createdirectorydialog.h"
#include "directorycreator.h"
#include "directoryeditordialog.h"
#include "directorycopyasdialog.h"
#include "directoryremover.h"
#include "scriptrunner.h"
#include "filecopier.h"

#include <QHeaderView>
#include <QDesktopServices>
#include <QSignalMapper>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>
#include <QDebug>


DataSourceView::DataSourceView(const QString &rootPath, QWidget *parent) :
    QTreeView(parent),
    m_path(rootPath)
{
    // boss requirement - icons shoudl have be at least 32px sized
    setStyleSheet("icon-size: 32px;");

    m_proxy = new DataSourceProxyModel(this);

    m_model = new DataSourceModel(this);
    m_proxy->setSourceModel(m_model);

    setModel(m_proxy);
    refreshModel();

    header()->close();

    setContextMenuPolicy(Qt::CustomContextMenu);

    m_signalMapper = new QSignalMapper(this);
    m_scriptRunner = new ScriptRunner(m_path, this);

    connect(m_signalMapper, SIGNAL(mappedString(QString)),
            this, SLOT(spawnZimaUtilityOnDir(QString)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(modelClicked(QModelIndex)));
    connect(MetadataCache::get(), SIGNAL(cleared()),
            this, SLOT(refreshModel()));
}

void DataSourceView::refreshModel()
{
    // it has to be reset here because calling QFileSystemModel's reset
    // or begin/end alternatives results in "/" as a root path
    setRootIndex(m_proxy->mapFromSource(m_model->setRootPath(m_path)));
}

void DataSourceView::modelClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    emit directorySelected(currentFileInfo().absoluteFilePath());
}

QFileInfo DataSourceView::currentFileInfo()
{
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return QFileInfo();

    QModelIndex srcIndex = m_proxy->mapToSource(index);
    return m_model->fileInfo(srcIndex);
}

void DataSourceView::addScriptsToContextMenu(QMenu *menu)
{
    auto fi = currentFileInfo();

    QDir dsDir(m_path + "/" + SCRIPT_DIR);
    QDir localDir(fi.absoluteFilePath() + "/" + SCRIPT_DIR);

    if (!dsDir.exists() && !localDir.exists())
        return;

    auto globalScripts = dsDir.entryInfoList(QDir::Files | QDir::Executable);
    auto localScripts = localDir.entryInfoList(QDir::Files | QDir::Executable);

    if (globalScripts.empty() && localScripts.empty())
        return;

    auto submenu = menu->addMenu(QIcon(":/gfx/arrow-right.png"), tr("Scripts..."));

    foreach (auto script, globalScripts) {
        submenu->addAction(script.fileName(), [=]() {
            this->runScriptOnDir(fi, script);
        });
    }

    foreach (auto script, localScripts) {
        submenu->addAction(script.fileName(), [=]() {
            this->runScriptOnDir(fi, script);
        });
    }

    menu->addSeparator();
}

void DataSourceView::showContextMenu(const QPoint &point)
{
    QModelIndex i = currentIndex();

    if (!i.isValid())
        return;

    QMenu *menu = new QMenu(this);

    menu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Open"), this, SLOT(indexOpenPath()));
    menu->addAction(QIcon(":/gfx/tab-new.png"), tr("Open in a new tab"), this, SLOT(openInANewTab()));
    menu->addAction(QIcon(":/gfx/gohome.png"), tr("Set as working directory"), this, SLOT(setWorkingDirectory()));
    menu->addAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Create directory"), this, SLOT(createDirectory()));

    menu->addSeparator();

    menu->addAction(QIcon(":/gfx/document-edit.png"), tr("Edit"), this, SLOT(editDirectory()));
    menu->addAction(QIcon(":/gfx/edit-copy.png"), tr("Copy as..."), this, SLOT(copyDirectoryAs()));
    menu->addAction(QIcon(":/gfx/list-remove.png"), tr("Delete"), this, SLOT(deleteDirectory()));

    menu->addSeparator();

    addScriptsToContextMenu(menu);

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

void DataSourceView::indexOpenPath()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(currentFileInfo().absoluteFilePath()));
}

void DataSourceView::openInANewTab()
{
    emit openInANewTabRequested(currentFileInfo().absoluteFilePath());
}

void DataSourceView::spawnZimaUtilityOnDir(const QString &label)
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

void DataSourceView::setWorkingDirectory()
{
    Settings::get()->setWorkingDir(currentFileInfo().absoluteFilePath());
    emit workingDirChanged();
}

void DataSourceView::createDirectory()
{
    QFileInfo fi = currentFileInfo();
    CreateDirectoryDialog dlg(fi.absoluteFilePath());

    if (dlg.exec() != QDialog::Accepted)
        return;

    if (QFile::exists(fi.absoluteFilePath() + "/" + dlg.name()))
    {
        QMessageBox::warning(
            this,
            tr("Directory exists"),
            tr("Directory %1 already exists.").arg(dlg.name())
        );
        return;
    }

    auto creator = new DirectoryCreator(fi.absoluteFilePath(), dlg.name(), this);

    if (dlg.hasPrototype())
        creator->setPrototype(dlg.prototype());

    creator->work();
}

void DataSourceView::editDirectory()
{
    QFileInfo fi = currentFileInfo();

    DirectoryEditorDialog dlg(fi, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        dlg.apply();
        emit directoryChanged(dlg.directoryPath());
    }
}

void DataSourceView::copyDirectoryAs()
{
    QFileInfo fi = currentFileInfo();
    QString dstDir;

    do {
        DirectoryCopyAsDialog dlg(fi, this);

        if (dlg.exec() != QDialog::Accepted)
            return;

        dstDir = dlg.directoryPath();

        if (!QFile::exists(dstDir))
            break;

        QMessageBox::warning(this, tr("Directory exists"), tr("Directory '%1' already exists.").arg(dstDir));
        dstDir.clear();
    } while (dstDir.isEmpty());

    qDebug() << "Copy" << fi.absoluteFilePath() << "as" << dstDir;

    FileCopier *cp = new FileCopier(this);
    cp->setMessage(tr("Please wait while the directory is being copied..."));
    cp->setDestination(dstDir);
    cp->setStopOnError(false);
    cp->addSourceDirectoryContents(fi);
    cp->work();

    emit directoryChanged(dstDir);
}

void DataSourceView::deleteDirectory()
{
    QFileInfo fi = currentFileInfo();

    if( QMessageBox::question(this,
                              tr("Do you really want to delete selected directory?"),
                              tr("Do you really want to delete directory '%1'? This action is irreversible.").arg(fi.absoluteFilePath()),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            ==  QMessageBox::Yes)
    {
        DirectoryRemover *rm = new DirectoryRemover(fi, this);
        rm->setMessage(tr("Please wait while the directory is being removed..."));
        rm->work();
    }
}

void DataSourceView::runScriptOnDir(const QFileInfo &dir, const QFileInfo &script)
{
    qDebug() << "Run" << script << "on" << dir;

    m_scriptRunner->run(script, dir);
}

bool DataSourceView::navigateToDirectory(const QString &path)
{
    // find the common root path. Then ise the index.
    // note: all QFileSystemModels have the index(path) so we need to
    // handle prefixes. ::match() did not work here.
    QString root = m_model->filePath(m_proxy->mapToSource(rootIndex()));
    if (!path.startsWith(root))
        return false;

    auto index = m_proxy->mapFromSource(m_model->index(path));
    setCurrentIndex(index);
    setExpanded(index, true);

    return true;
}
