#include <QEvent>
#include "datasourcewidget.h"
#include "datasourceview.h"
#include "settings.h"
#include "datasourcehistory.h"
#include "directoryeditordialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMenu>
#include <QStyle>
#include <QUrl>
#include <QtDebug>
#include <QShowEvent>
#include <QTimer>


DataSourceWidget::DataSourceWidget(const QString &dir, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    connect(dirWidget, &DirectoryWidget::aiReferencesRequested, this, &DataSourceWidget::aiReferencesRequested);


    splitter->setSizes(Settings::get()->ServersSplitterSizes);

    connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));
    connect(dirWidget, SIGNAL(changeSettings()), this, SLOT(settingsChanged()));
    connect(dirWidget, SIGNAL(prepareFileOperation()), this, SLOT(releaseFileSystemModels()));
    connect(dirWidget, SIGNAL(fileOperationFinished()), this, SLOT(restoreFileSystemModels()));
    connect(dirWidget, SIGNAL(refreshRequested()), this, SIGNAL(refreshRequested()));
    connect(dirWidget, SIGNAL(openDirectoryRequested(QString)),
            this, SLOT(openDirectoryFromWebView(QString)));
    connect(dsList, SIGNAL(pageContextMenuRequested(int,QPoint)),
            this, SLOT(showDataSourceContextMenu(int,QPoint)));
    connect(dsList, SIGNAL(pageActivated(int)),
            this, SLOT(openDataSourceRoot(int)));
    connect(MetadataCache::get(), SIGNAL(cleared()),
            this, SLOT(refreshDataSourceMetadata()));

    m_history = new DataSourceHistory(this);

    connect(m_history, SIGNAL(openDirectory(QString)),
            this, SLOT(setDirectory(QString)));

    m_currentDir = dir;
}

void DataSourceWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_initialized || m_initializationQueued)
        return;
    m_initializationQueued = true;
    QTimer::singleShot(0, this, [this] {
        m_initializationQueued = false;
        if (m_initialized || !isVisible())
            return;
        m_initialized = true;
        setupDataSources(m_currentDir);
        dirWidget->settingsChanged();
    });
}

void DataSourceWidget::splitterMoved(int, int)
{
    Settings::get()->ServersSplitterSizes = splitter->sizes();
}

void DataSourceWidget::handleOpenPartDirectory(const QFileInfo &fi)
{
    setDirectory(fi.absoluteFilePath());
}

void DataSourceWidget::openDirectoryFromWebView(const QString &path)
{
    if (!QFileInfo(path).isDir())
        return;

    setDirectory(path);
    m_history->track(path);
    announceDirectoryChange(path);
}

void DataSourceWidget::announceDirectoryChange(const QString &dir)
{
    m_currentDir = dir;
    emit directoryChanged(this, dir);
}

void DataSourceWidget::settingsChanged()
{
    if (!m_initialized)
        return;
    // Data sources
    if (Settings::get()->DataSourcesNeedsUpdate)
    {
        m_dataSources.clear();

        // firstly delete all stuff used. Remember "the reset"
        while (dsList->count())
        {
            dsList->widget(0)->deleteLater();
            dsList->removePage(0);
        }

        qApp->processEvents();

        // now setup all item==group again
        setupDataSources(m_currentDir);
    }

    dirWidget->settingsChanged();
}

void DataSourceWidget::expand(const QModelIndex & index)
{
    qobject_cast<QTreeView*>(dsList->currentWidget())->expand(index);
}

void DataSourceWidget::releaseFileSystemModels()
{
    for (int i = 0; i < dsList->count(); ++i)
    {
        DataSourceView *view = qobject_cast<DataSourceView*>(dsList->widget(i));
        if (view)
            view->releaseFileSystemModel();
    }

    qApp->processEvents();
}

void DataSourceWidget::restoreFileSystemModels()
{
    for (int i = 0; i < dsList->count(); ++i)
    {
        DataSourceView *view = qobject_cast<DataSourceView*>(dsList->widget(i));
        if (view)
            view->restoreFileSystemModelState();
    }

    qApp->processEvents();
}

QModelIndex DataSourceWidget::currentIndex()
{
    return qobject_cast<QTreeView*>(dsList->currentWidget())->currentIndex();
}

void DataSourceWidget::setDirectory(const QString &path)
{
    if (!m_initialized) {
        m_currentDir = path;
        return;
    }
    for (int i = 0; i < dsList->count(); ++i)
    {
        DataSourceView *w = qobject_cast<DataSourceView*>(dsList->widget(i));
        if (w->navigateToDirectory(path))
        {
            dsList->setCurrentIndex(i);
            dirWidget->setDirectory(path);
            m_currentDir = path;
        }
    }
}

DataSourceHistory *DataSourceWidget::history()
{
    return m_history;
}

QString DataSourceWidget::currentDir() const
{
    return m_currentDir;
}

DataSource *DataSourceWidget::dataSourceForPage(int index) const
{
    DataSourceView *view = qobject_cast<DataSourceView*>(dsList->widget(index));
    if (!view)
        return 0;

    return m_dataSources.value(view, 0);
}

QString DataSourceWidget::dataSourceLabel(DataSource *dataSource) const
{
    QString label = MetadataCache::get()->label(dataSource->rootPath);
    return label.isEmpty() ? dataSource->name : label;
}

void DataSourceWidget::goToWorkingDirectory()
{
    QString wdir = Settings::get()->getWorkingDir();
    setDirectory(wdir);
    m_history->track(wdir);
}

void DataSourceWidget::openAboutPage()
{
    dirWidget->openAboutPage();
}

void DataSourceWidget::showDataSourceContextMenu(int index, const QPoint &globalPos)
{
    DataSource *dataSource = dataSourceForPage(index);
    if (!dataSource)
        return;

    dsList->setCurrentIndex(index);
    const QString rootPath = dataSource->rootPath;
    const bool hasDirectory = QFileInfo(rootPath).isDir();

    QMenu menu(this);
    QAction *openAction = menu.addAction(
        style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Open")
    );
    openAction->setEnabled(hasDirectory);
    QAction *openInNewTabAction = menu.addAction(
        QIcon(":/gfx/tab-new.png"),
        tr("Open in a new tab")
    );
    openInNewTabAction->setEnabled(hasDirectory);
    auto aiAction = menu.addAction(QIcon(":/gfx/navigation/terminal.svg"), tr("Add to AI question"));
    aiAction->setObjectName("addToAiQuestion");
    aiAction->setEnabled(hasDirectory);
    QAction *workingDirectoryAction = menu.addAction(
        QIcon(":/gfx/gohome.png"), tr("Set as working directory")
    );
    workingDirectoryAction->setEnabled(hasDirectory);
    menu.addSeparator();

    QAction *editAction = menu.addAction(
        QIcon(":/gfx/document-edit.png"),
        tr("Data source properties")
    );
    editAction->setEnabled(hasDirectory);

    QAction *settingsAction = menu.addAction(
        QIcon(":/gfx/configure.png"),
        tr("Edit in settings")
    );

    QAction *selectedAction = menu.exec(globalPos);

    if (selectedAction == openAction && QFileInfo(rootPath).isDir())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(rootPath));
    }
    else if (selectedAction == workingDirectoryAction && QFileInfo(rootPath).isDir())
    {
        Settings::get()->setWorkingDir(rootPath);
        emit workingDirChanged();
    }
    else if (selectedAction == openInNewTabAction)
    {
        emit openInANewTabRequested(rootPath);
    }
    else if (selectedAction == aiAction)
    {
        emit aiReferencesRequested({rootPath});
    }
    else if (selectedAction == editAction)
    {
        DirectoryEditorDialog dlg(
            QFileInfo(dataSource->rootPath),
            this,
            DirectoryEditorDialog::DataSourceRoot
        );

        if (dlg.exec() == QDialog::Accepted)
        {
            dlg.apply();
            MetadataCache::get()->clear();
        }
    }
    else if (selectedAction == settingsAction)
    {
        emit editDataSourceRequested(dataSource);
    }
}

void DataSourceWidget::openDataSourceRoot(int index)
{
    DataSource *dataSource = dataSourceForPage(index);
    if (!dataSource || !QFileInfo(dataSource->rootPath).isDir())
        return;

    DataSourceView *view = qobject_cast<DataSourceView*>(dsList->widget(index));
    if (!view || !view->navigateToDirectory(dataSource->rootPath))
        return;

    dirWidget->setDirectory(dataSource->rootPath);
    m_history->track(dataSource->rootPath);
    announceDirectoryChange(dataSource->rootPath);
}

void DataSourceWidget::refreshDataSourceMetadata()
{
    DataSourceIconProvider iconProvider;

    for (int i = 0; i < dsList->count(); ++i)
    {
        DataSource *dataSource = dataSourceForPage(i);
        if (!dataSource)
            continue;

        dataSource->icon = iconProvider.icon(QFileInfo(dataSource->rootPath));
        dsList->setPageText(i, dataSourceLabel(dataSource));
        dsList->setPageIcon(i, dataSource->icon);
    }

    if (!m_currentDir.isEmpty())
        dirWidget->updateDirectory(m_currentDir);
}

void DataSourceWidget::setupDataSources(const QString &dir)
{
    DataSourceIconProvider iconProvider;

    foreach(DataSource *ds, Settings::get()->DataSources)
    {
        DataSourceView *view = new DataSourceView(ds->rootPath, this);
        connect(view, &DataSourceView::aiReferencesRequested, this, &DataSourceWidget::aiReferencesRequested);

        connect(view, SIGNAL(showSettings(SettingsDialog::Section)),
                this, SIGNAL(showSettings(SettingsDialog::Section)));
        connect(view, SIGNAL(workingDirChanged()),
                this, SIGNAL(workingDirChanged()));
        connect(view, SIGNAL(directorySelected(QString)),
                dirWidget, SLOT(setDirectory(QString)));
        connect(dirWidget, SIGNAL(openPartDirectory(QFileInfo)),
                this, SLOT(handleOpenPartDirectory(QFileInfo)));
        connect(view, SIGNAL(directorySelected(QString)),
                m_history, SLOT(track(QString)));
        connect(view, SIGNAL(directorySelected(QString)),
                this, SLOT(announceDirectoryChange(QString)));
        connect(view, SIGNAL(directoryChanged(QString)),
                dirWidget, SLOT(updateDirectory(QString)));
        connect(view, SIGNAL(openInANewTabRequested(QString)),
                this, SIGNAL(openInANewTabRequested(QString)));

        ds->icon = iconProvider.icon(QFileInfo(ds->rootPath));

        m_dataSources.insert(view, ds);
        dsList->addPage(view, dataSourceLabel(ds), ds->icon);
    }

    dsList->setVisibleRows(dsList->count());

    setDirectory(dir);
    m_history->track(dir);
}

void DataSourceWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
