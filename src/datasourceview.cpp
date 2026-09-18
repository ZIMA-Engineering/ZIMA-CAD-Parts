#include "directoryprotection.h"
#include "datasourceview.h"
#include "datasourcemodel.h"
#include "partstoolsdialog.h"
#include "settings.h"
#include "settingsdialog.h"
#include "createdirectorydialog.h"
#include "directorycreator.h"
#include "directoryeditordialog.h"
#include "directorycopyasdialog.h"
#include "directoryremover.h"
#include "metadata.h"
#include "partcache.h"
#include "scriptrunner.h"
#include "filecopier.h"

#include <QHeaderView>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QItemSelectionModel>
#include <QUrl>
#include <QDebug>

#include <algorithm>


DataSourceView::DataSourceView(const QString &rootPath, QWidget *parent) :
    QTreeView(parent),
    m_path(rootPath),
    m_verticalScrollBeforeReset(0),
    m_horizontalScrollBeforeReset(0),
    m_hasModelState(false)
{
    // boss requirement - icons shoudl have be at least 32px sized
    setStyleSheet("icon-size: 32px;");

    setupModel();
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setDefaultDropAction(Qt::CopyAction);

    header()->close();

    setContextMenuPolicy(Qt::CustomContextMenu);

    m_scriptRunner = new ScriptRunner(m_path, this);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(modelClicked(QModelIndex)));
    connect(MetadataCache::get(), SIGNAL(cleared()),
            this, SLOT(refreshModel()));
}

void DataSourceView::setupModel()
{
    m_proxy = new DataSourceProxyModel(this);
    m_model = new DataSourceModel(this);
    m_proxy->setSourceModel(m_model);

    setModel(m_proxy);
    refreshModel();
}

void DataSourceView::releaseFileSystemModel()
{
    saveFileSystemModelState();

    QItemSelectionModel *oldSelectionModel = selectionModel();
    setModel(nullptr);
    if (oldSelectionModel && oldSelectionModel != selectionModel())
        delete oldSelectionModel;

    delete m_proxy;
    delete m_model;
    m_proxy = nullptr;
    m_model = nullptr;

    setupModel();
}

void DataSourceView::restoreFileSystemModelState()
{
    if (!m_hasModelState || !m_model || !m_proxy)
        return;

    QStringList expandedPaths = m_expandedPathsBeforeReset;
    QString currentPath = m_currentPathBeforeReset;
    int verticalScroll = m_verticalScrollBeforeReset;
    int horizontalScroll = m_horizontalScrollBeforeReset;

    m_expandedPathsBeforeReset.clear();
    m_currentPathBeforeReset.clear();
    m_hasModelState = false;

    expandedPaths.removeDuplicates();
    std::sort(expandedPaths.begin(), expandedPaths.end(), [](const QString &a, const QString &b) {
        return a.length() < b.length();
    });

    QStringList pending = expandedPaths;

    for (int attempt = 0; attempt < 3 && !pending.isEmpty(); ++attempt)
    {
        QStringList remaining;

        foreach (const QString &path, pending)
        {
            if (!expandPath(path))
                remaining << path;
        }

        pending = remaining;

        if (!pending.isEmpty())
            qApp->processEvents();
    }

    currentPath = nearestExistingPath(currentPath);
    if (!currentPath.isEmpty())
        navigateToDirectory(currentPath);

    if (verticalScrollBar())
        verticalScrollBar()->setValue(verticalScroll);

    if (horizontalScrollBar())
        horizontalScrollBar()->setValue(horizontalScroll);
}

void DataSourceView::refreshModel()
{
    if (!m_model || !m_proxy)
        return;

    // it has to be reset here because calling QFileSystemModel's reset
    // or begin/end alternatives results in "/" as a root path
    setRootIndex(m_proxy->mapFromSource(m_model->setRootPath(m_path)));
}

void DataSourceView::saveFileSystemModelState()
{
    m_expandedPathsBeforeReset.clear();
    m_currentPathBeforeReset.clear();
    m_verticalScrollBeforeReset = verticalScrollBar() ? verticalScrollBar()->value() : 0;
    m_horizontalScrollBeforeReset = horizontalScrollBar() ? horizontalScrollBar()->value() : 0;
    m_hasModelState = false;

    if (!m_model || !m_proxy)
        return;

    QModelIndex current = currentIndex();
    if (current.isValid())
        m_currentPathBeforeReset = m_model->filePath(m_proxy->mapToSource(current));

    collectExpandedPaths(rootIndex());
    m_hasModelState = true;
}

void DataSourceView::collectExpandedPaths(const QModelIndex &parent)
{
    int rows = m_proxy->rowCount(parent);

    for (int row = 0; row < rows; ++row)
    {
        QModelIndex child = m_proxy->index(row, 0, parent);

        if (!child.isValid() || !isExpanded(child))
            continue;

        m_expandedPathsBeforeReset << m_model->filePath(m_proxy->mapToSource(child));
        collectExpandedPaths(child);
    }
}

bool DataSourceView::expandPath(const QString &path)
{
    if (!QFileInfo(path).isDir() || !pathBelongsToRoot(path))
        return true;

    QModelIndex index = m_proxy->mapFromSource(m_model->index(path));

    if (!index.isValid())
        return false;

    setExpanded(index, true);
    return true;
}

bool DataSourceView::pathBelongsToRoot(const QString &path) const
{
    const QString cleanRoot = QDir::cleanPath(m_path);
    const QString cleanPath = QDir::cleanPath(path);

#ifdef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif

    return cleanPath.compare(cleanRoot, sensitivity) == 0
            || cleanPath.startsWith(cleanRoot + QLatin1Char('/'), sensitivity);
}

QString DataSourceView::nearestExistingPath(const QString &path) const
{
    if (path.isEmpty())
        return QString();

    QString current = QDir::cleanPath(path);

    while (!current.isEmpty())
    {
        QFileInfo fi(current);

        if (fi.isDir() && pathBelongsToRoot(current))
            return current;

        QString parentPath = QDir::cleanPath(QFileInfo(current).absolutePath());
        if (parentPath == current)
            break;

        current = parentPath;
    }

    return QFileInfo(m_path).isDir() ? QDir::cleanPath(m_path) : QString();
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

    QDir::Filters scriptFilters = QDir::Files;
#ifndef Q_OS_WIN
    scriptFilters |= QDir::Executable;
#endif

    auto globalScripts = dsDir.entryInfoList(scriptFilters);
    auto localScripts = localDir.entryInfoList(scriptFilters);

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
    const auto clicked = indexAt(point);
    if (clicked.isValid()) setCurrentIndex(clicked);
    QModelIndex i = currentIndex();

    if (!i.isValid())
        return;

    QMenu *menu = new QMenu(this);

    menu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Open"), this, SLOT(indexOpenPath()));
    menu->addAction(QIcon(":/gfx/tab-new.png"), tr("Open in a new tab"), this, SLOT(openInANewTab()));
    auto ai = menu->addAction(QIcon(":/gfx/navigation/terminal.svg"), tr("Add to AI question"));
    ai->setObjectName("addToAiQuestion");
    const auto reference = currentFileInfo().absoluteFilePath();
    connect(ai, &QAction::triggered, this, [this, reference] { emit aiReferencesRequested({reference}); });
    menu->addAction(QIcon(":/gfx/gohome.png"), tr("Set as working directory"), this, SLOT(setWorkingDirectory()));
    menu->addAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Create directory"), this, SLOT(createDirectory()));

    menu->addSeparator();

    menu->addAction(QIcon(":/gfx/document-edit.png"), tr("Directory properties"), this, SLOT(editDirectory()));
    menu->addAction(QIcon(":/gfx/edit-copy.png"), tr("Copy as..."), this, SLOT(copyDirectoryAs()));
    auto deleteAction = menu->addAction(QIcon(":/gfx/list-remove.png"), tr("Delete"), this, SLOT(deleteDirectory()));
    const auto locked = DirectoryProtection::removalLock(currentFileInfo());
    deleteAction->setEnabled(locked.isEmpty());
    if (!locked.isEmpty()) {
        deleteAction->setToolTip(DirectoryProtection::message(locked));
        menu->setToolTipsVisible(true);
    }

    menu->addSeparator();

    addScriptsToContextMenu(menu);

    for (const auto &tool : {QString("ptc-clean"), QString("ps2pdf"), QString("step-edit")}) {
        const QString icon = tool == "ptc-clean" ? "ZIMA-PTC-Cleaner" : tool == "ps2pdf" ? "ZIMA-PS2PDF" : "ZIMA-STEP-Edit";
        auto action = menu->addAction(QIcon(":/gfx/external_programs/" + icon + ".png"), PartsToolsDialog::title(tool));
        const auto directory = currentFileInfo().absoluteFilePath();
        connect(action, &QAction::triggered, this, [this, tool, directory] {
            if (tool == "ptc-clean" || tool == "ps2pdf") {
                auto dialog = new PartsToolsDialog(tool, directory, this);
                dialog->setAttribute(Qt::WA_DeleteOnClose);
                connect(dialog, &PartsToolsDialog::filesChanged, this, [this, directory] { emit directoryChanged(directory); });
                dialog->show();
            } else {
                PartsToolsDialog dialog(tool, directory, this);
                connect(&dialog, &PartsToolsDialog::filesChanged, this, [this, directory] { emit directoryChanged(directory); });
                dialog.exec();
            }
        });
    }

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
    const auto locked = DirectoryProtection::removalLock(fi);
    if (!locked.isEmpty()) {
        QMessageBox::warning(this, tr("Directory locked"), DirectoryProtection::message(locked));
        return;
    }

    if( QMessageBox::question(this,
                              tr("Do you really want to delete selected directory?"),
                              tr("Do you really want to delete directory '%1'? This action is irreversible.").arg(fi.absoluteFilePath()),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            ==  QMessageBox::Yes)
    {
        MetadataCache::get()->clearBelow(fi.absoluteFilePath());
        PartCache::get()->clearBelow(fi.absoluteFilePath());
        DirectoryRemover *rm = new DirectoryRemover(fi, this);
        rm->setMessage(tr("Please wait while the directory is being removed..."));
        rm->work();
        rm->deleteLater();
        if (!QFileInfo::exists(fi.absoluteFilePath())) {
            const QString parentPath = fi.absolutePath();
            // QFileSystemModel observes the actual removal and updates that
            // branch. Keep its root and expanded siblings intact.
            if (navigateToDirectory(parentPath))
                emit directorySelected(parentPath);
            MetadataCache::get()->clearBelow(fi.absoluteFilePath());
            PartCache::get()->clear(fi.absoluteFilePath());
        }
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
    const QString relative = QDir(root).relativeFilePath(path);
    if (relative == ".." || relative.startsWith("../") || QDir::isAbsolutePath(relative))
        return false;

    auto index = m_proxy->mapFromSource(m_model->index(path));
    if (!index.isValid())
        return false;
    setCurrentIndex(index);
    setExpanded(index, true);

    return true;
}
