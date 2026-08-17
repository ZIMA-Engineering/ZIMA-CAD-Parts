#include <QDir>
#include <QWebEngineHistory>
#include <QtDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QProcess>
#include <QToolButton>

#include "directorywidget.h"
#include "ui_directorywidget.h"
#include "browserpage.h"
#include "directorywebview.h"
#include "filemodel.h"
#include "filefiltermodel.h"
#include "settings.h"
#include "filtersdialog.h"
#include "partsdeletedialog.h"
#include "partcache.h"
#include "extensions/productview/productview.h"


DirectoryWidget::DirectoryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectoryWidget)
{
    ui->setupUi(this);
    ui->partsWebView->setPage(new BrowserPage(ui->partsWebView));

    m_productView = new ProductView(this);
    m_directoryIndexMenu = new QMenu(this);
    m_partsIndexMenu = new QMenu(this);

    ui->dirWebViewPinButton->setPopupMode(QToolButton::MenuButtonPopup);
    ui->partsIndexPinButton->setPopupMode(QToolButton::MenuButtonPopup);

    connect(ui->refreshButton, SIGNAL(clicked()),
            this, SLOT(refreshButton_clicked()));

    ui->dirWebViewBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->dirWebViewForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->dirWebViewReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->dirWebViewGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
    ui->dirWebViewPinButton->setFixedWidth(
        qMax(ui->dirWebViewGoButton->sizeHint().width(),
             ui->dirWebViewEditButton->sizeHint().width())
    );

    connect(ui->dirWebViewBackButton, SIGNAL(clicked()),
            ui->dirWebView, SLOT(back()));
    connect(ui->dirWebViewForwardButton, SIGNAL(clicked()),
            ui->dirWebView, SLOT(forward()));
    connect(ui->dirWebViewReloadButton, SIGNAL(clicked()),
            this, SLOT(dirWebViewReloadButton_clicked()));
    connect(ui->dirWebViewUrlLineEdit, SIGNAL(returnPressed()),
            this, SLOT(dirWebViewUrlLineEdit_returnPressed()));
    connect(ui->dirWebViewGoButton, SIGNAL(clicked()),
            this, SLOT(dirWebViewGoButton_clicked()));
    connect(ui->dirWebViewPinButton, SIGNAL(clicked()),
            this, SLOT(dirWebViewPinButton_clicked()));
    connect(ui->dirWebViewEditButton, SIGNAL(clicked()),
            this, SLOT(dirWebViewEditButton_clicked()));
    connect(ui->dirWebView, SIGNAL(urlChanged(QUrl)),
            this, SLOT(dirWebView_urlChanged(QUrl)));
    connect(ui->dirWebView, SIGNAL(openDirectoryRequested(QString)),
            this, SIGNAL(openDirectoryRequested(QString)));
    connect(&m_autoIndexWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(watchedAutoIndexDirectoryChanged(QString)));
    connect(PartCache::get(), SIGNAL(directoryChanged(QString)),
            this, SLOT(cachedDirectoryChanged(QString)));

    ui->partsIndexBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->partsIndexForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->partsIndexReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->partsIndexGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
    ui->partsIndexPinButton->setFixedWidth(
        qMax(ui->partsIndexGoButton->sizeHint().width(),
             ui->partsIndexEditButton->sizeHint().width())
    );

    connect(ui->partsIndexBackButton, SIGNAL(clicked()),
            ui->partsWebView, SLOT(back()));
    connect(ui->partsIndexForwardButton, SIGNAL(clicked()),
            ui->partsWebView, SLOT(forward()));
    connect(ui->partsIndexReloadButton, SIGNAL(clicked()),
            ui->partsWebView, SLOT(reload()));
    connect(ui->partsIndexUrlLineEdit, SIGNAL(returnPressed()),
            this, SLOT(partsIndexUrlLineEdit_returnPressed()));
    connect(ui->partsIndexGoButton, SIGNAL(clicked()),
            this, SLOT(partsIndexGoButton_clicked()));
    connect(ui->partsIndexPinButton, SIGNAL(clicked()),
            this, SLOT(partsIndexPinButton_clicked()));
    connect(ui->partsIndexEditButton, SIGNAL(clicked()),
            this, SLOT(partsIndexEditButton_clicked()));
    connect(ui->partsWebView, SIGNAL(urlChanged(QUrl)),
            this, SLOT(partsWebView_urlChanged(QUrl)));

    connect(ui->copyToWorkingDirButton, SIGNAL(clicked()),
            ui->partsTreeView, SLOT(copyToWorkingDir()));
    connect(ui->moveButton, SIGNAL(clicked()),
            this, SLOT(moveSelectedParts()));
    connect(ui->btnDelete, SIGNAL(clicked()),
            this, SLOT(deleteSelectedParts()));
    connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(adjustThumbColumnWidth(int)));

    connect(ui->partsTreeView, SIGNAL(previewProductView(QFileInfo)),
            this, SLOT(previewInProductView(QFileInfo)));
    connect(ui->partsTreeView, SIGNAL(hideProductView()),
            m_productView, SLOT(hide()));
    connect(ui->partsTreeView, SIGNAL(openPartDirectory(QFileInfo)),
            this, SIGNAL(openPartDirectory(QFileInfo)));

    connect(ui->filterButton, SIGNAL(clicked()),
            this, SLOT(setFiltersDialog()));

    ui->dirWebView->loadAboutPage();
}

DirectoryWidget::~DirectoryWidget()
{
    delete ui;
}

void DirectoryWidget::setDirectory(const QString &rootPath)
{
    if (rootPath.trimmed().isEmpty())
    {
        m_currentRootPath.clear();
        watchAutoIndexDirectory(QString());
        ui->partsWebView->hide();
        ui->dirWebView->loadAboutPage();
        updateIndexMenus();
        return;
    }

    setEnabled(false);
    m_currentRootPath = rootPath;
    watchAutoIndexDirectory(rootPath);

    // set the directory to the file model
    ui->partsTreeView->setDirectory(rootPath);
    // handle the ui->partsWebView, custom index-parts*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->partsWebView, "index-parts", true, false);
    // handle the ui->partsWebView, custom index*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->dirWebView, "index", false, true);
    updateIndexMenus();

    setEnabled(true);
}

void DirectoryWidget::updateDirectory(const QString &rootPath)
{
    if (m_currentRootPath.compare(rootPath) == 0)
        reloadDirectoryIndex();

    if (ui->partsTreeView->currentPath().compare(rootPath) == 0)
        ui->partsTreeView->directoryChanged();
}

void DirectoryWidget::openAboutPage()
{
    ui->tabWidget->setCurrentIndex(0); // Switch to web view
    ui->dirWebView->loadAboutPage();
}

void DirectoryWidget::loadIndexHtml(const QString &rootPath, QWebEngineView *webView, const QString &filterBase, bool hideIfNotFound, bool allowAutoIndex)
{
    QFileInfoList indexes;

    foreach (const QFileInfo &file, indexFiles(rootPath, filterBase))
    {
        if (file.isReadable())
            indexes << file;
    }

    if (indexes.isEmpty())
    {
        if (allowAutoIndex && filterBase == "index")
        {
            DirectoryWebView *dirView = qobject_cast<DirectoryWebView *>(webView);

            if (dirView && MetadataCache::get()->autoIndexEnabled(rootPath))
            {
                webView->show();
                dirView->loadAutoIndexPage(rootPath);
                return;
            }
        }

        webView->setHtml("");
        if (hideIfNotFound) webView->hide();
        // load aboutPage only when there is no custom index.html and there is no WD specified
        if (rootPath == DEFAULT_WDIR && webView == ui->dirWebView)
        {
            ui->dirWebView->loadAboutPage();
            return;
        }
        QDir d(rootPath);
        if (!d.cdUp())
            return;
        loadIndexHtml(d.absolutePath(), webView, filterBase, hideIfNotFound, false);
        return;
    }

    QFileInfo selectedIndex = selectedIndexFile(indexes);

    webView->show();

    if (DirectoryWebView *dirView = qobject_cast<DirectoryWebView *>(webView))
        dirView->setRootPath(rootPath);

    webView->load(QUrl::fromLocalFile(selectedIndex.absoluteFilePath()));
}

QStringList DirectoryWidget::indexNameFilters(const QString &filterBase) const
{
    return QStringList()
            << filterBase + "_??.html"
            << filterBase + "_??.htm"
            << filterBase + ".html"
            << filterBase + ".htm";
}

QFileInfoList DirectoryWidget::indexFiles(const QString &rootPath, const QString &filterBase) const
{
    QDir dir(rootPath + "/" + METADATA_DIR);
    return dir.entryInfoList(indexNameFilters(filterBase),
                             QDir::Files,
                             QDir::Name);
}

QFileInfo DirectoryWidget::selectedIndexFile(const QFileInfoList &files) const
{
    if (files.isEmpty())
        return QFileInfo();

    QFileInfo selected = files.first();
    QString language = Settings::get()->getCurrentLanguageCode().left(2);

    foreach (const QFileInfo &file, files)
    {
        QString prefix = file.completeBaseName();

        if (prefix.lastIndexOf('_') == prefix.length() - 3
                && prefix.right(2) == language)
        {
            selected = file;
        }
    }

    return selected;
}

void DirectoryWidget::updateIndexMenus()
{
    updateIndexMenu(ui->dirWebViewPinButton, m_directoryIndexMenu, "index");
    updateIndexMenu(ui->partsIndexPinButton, m_partsIndexMenu, "index-parts");
}

void DirectoryWidget::updateIndexMenu(QToolButton *button, QMenu *menu, const QString &filterBase)
{
    menu->clear();
    button->setMenu(nullptr);

    QFileInfoList files = indexFiles(m_currentRootPath, filterBase);
    if (files.isEmpty())
        return;

    QAction *deleteAction = menu->addAction(
        QIcon(":/gfx/list-remove.png"),
        tr("Delete index")
    );
    connect(deleteAction, &QAction::triggered, this, [this, filterBase]() {
        deleteIndexFiles(filterBase, false);
    });

    if (files.count() > 1)
    {
        QAction *deleteAllAction = menu->addAction(
            QIcon(":/gfx/list-remove.png"),
            tr("Delete all indexes")
        );
        connect(deleteAllAction, &QAction::triggered, this, [this, filterBase]() {
            deleteIndexFiles(filterBase, true);
        });
    }

    button->setMenu(menu);
}

void DirectoryWidget::deleteIndexFiles(const QString &filterBase, bool deleteAll)
{
    QFileInfoList files = indexFiles(m_currentRootPath, filterBase);
    if (files.isEmpty())
    {
        updateIndexMenus();
        return;
    }

    QFileInfoList targets;

    if (deleteAll)
    {
        targets = files;
    }
    else
    {
        QFileInfoList readableFiles;

        foreach (const QFileInfo &file, files)
        {
            if (file.isReadable())
                readableFiles << file;
        }

        QFileInfo selected = selectedIndexFile(readableFiles);

        if (!selected.exists())
            selected = selectedIndexFile(files);

        if (selected.exists())
            targets << selected;
    }

    if (targets.isEmpty())
    {
        updateIndexMenus();
        return;
    }

    QStringList paths;
    foreach (const QFileInfo &file, targets)
        paths << QDir::toNativeSeparators(file.absoluteFilePath());

    QString question = targets.count() == 1
            ? tr("The following index file will be deleted:\n\n%1\n\nContinue?")
            : tr("The following index files will be deleted:\n\n%1\n\nContinue?");
    QString title = deleteAll ? tr("Delete all indexes?") : tr("Delete index?");

    if (QMessageBox::question(
                this,
                title,
                question.arg(paths.join("\n")),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            ) != QMessageBox::Yes)
    {
        return;
    }

    QStringList failures;

    foreach (const QFileInfo &file, targets)
    {
        if (!QFile::remove(file.absoluteFilePath()))
            failures << QDir::toNativeSeparators(file.absoluteFilePath());
    }

    if (!failures.isEmpty())
    {
        QMessageBox::warning(
            this,
            tr("Index deletion failed"),
            tr("The following index files could not be deleted:\n\n%1")
                .arg(failures.join("\n"))
        );
    }

    updateIndexMenus();

    if (filterBase == "index")
        reloadDirectoryIndex();
    else
        reloadPartsIndex();
}

void DirectoryWidget::watchAutoIndexDirectory(const QString &rootPath)
{
    QStringList watched = m_autoIndexWatcher.directories();

    if (!watched.isEmpty())
        m_autoIndexWatcher.removePaths(watched);

    if (!QFileInfo(rootPath).isDir())
        return;

    m_autoIndexWatcher.addPath(rootPath);

    QString metadataPath = rootPath + "/" + METADATA_DIR;

    if (QFileInfo(metadataPath).isDir())
        m_autoIndexWatcher.addPath(metadataPath);
}

void DirectoryWidget::reloadDirectoryIndex()
{
    if (m_currentRootPath.isEmpty())
        return;

    loadIndexHtml(m_currentRootPath, ui->dirWebView, "index", false, true);
}

void DirectoryWidget::reloadPartsIndex()
{
    if (m_currentRootPath.isEmpty())
        return;

    loadIndexHtml(m_currentRootPath, ui->partsWebView, "index-parts", true, false);
}

void DirectoryWidget::editIndexFile(const QString &path)
{
    QUrl url = QUrl::fromUserInput(path);
    QString editor = Settings::get()->TextEditorPath;

    if (editor.isEmpty()) {
        QDesktopServices::openUrl(url);
        return;
    }

    QStringList args;
    QString localPath = url.isLocalFile() ? url.toLocalFile() : url.toString();
    args << localPath;
    QProcess::startDetached(editor, args);
}

void DirectoryWidget::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        updateIndexMenus();
        if (ui->dirWebView->url().path().startsWith("/data/zima-cad-parts") )
            ui->dirWebView->loadAboutPage();
        break;
    default:
        break;
    }
}

void DirectoryWidget::settingsChanged()
{
    ui->dirWebViewDevWidget->setVisible(Settings::get()->DeveloperEnabled
                                        && Settings::get()->DeveloperDirWebViewToolBar);
    ui->partsIndexDeveloperWidget->setVisible(Settings::get()->DeveloperEnabled);
    ui->thumbnailSizeSlider->setValue(Settings::get()->GUIThumbWidth);

    ui->partsTreeView->settingsChanged();
}

void DirectoryWidget::dirWebViewUrlLineEdit_returnPressed()
{
    dirWebViewGoButton_clicked();
}

void DirectoryWidget::dirWebViewGoButton_clicked()
{
    QString str = ui->dirWebViewUrlLineEdit->text();

    if (str == "ZIMA-CAD-Parts:about")
    {
        ui->dirWebView->loadAboutPage();
    }
    else
        ui->dirWebView->setUrl(QUrl(str));
}

void DirectoryWidget::dirWebViewReloadButton_clicked()
{
    if (ui->dirWebView->isAutoIndexPage())
    {
        reloadDirectoryIndex();
        return;
    }

    ui->dirWebView->reload();
}

void DirectoryWidget::partsIndexUrlLineEdit_returnPressed()
{
    partsIndexGoButton_clicked();
}

void DirectoryWidget::partsIndexGoButton_clicked()
{
    QString str = ui->partsIndexUrlLineEdit->text();

    ui->partsWebView->load(QUrl(str));

    if (ui->partsWebView->isHidden())
        ui->partsWebView->show();
}

void DirectoryWidget::partsIndexPinButton_clicked()
{
    ui->partsTreeView->createIndexHtmlFile(ui->partsIndexUrlLineEdit->text(), "index-parts");
    updateIndexMenus();
}

void DirectoryWidget::partsIndexEditButton_clicked()
{
    editIndexFile(ui->partsIndexUrlLineEdit->text());
}

void DirectoryWidget::dirWebViewPinButton_clicked()
{
    ui->partsTreeView->createIndexHtmlFile(ui->dirWebViewUrlLineEdit->text(), "index");
    updateIndexMenus();
}

void DirectoryWidget::dirWebViewEditButton_clicked()
{
    editIndexFile(ui->dirWebViewUrlLineEdit->text());
}

void DirectoryWidget::refreshButton_clicked()
{
    emit refreshRequested();
}

void DirectoryWidget::dirWebView_urlChanged(const QUrl &url)
{
    ui->dirWebViewBackButton->setEnabled(ui->dirWebView->history()->canGoBack());
    ui->dirWebViewForwardButton->setEnabled(ui->dirWebView->history()->canGoForward());
    ui->dirWebViewEditButton->setEnabled(url.scheme() == "file"
                                         && !ui->dirWebView->isAutoIndexPage());
    ui->dirWebViewPinButton->setEnabled(!ui->dirWebView->isAutoIndexPage());

    QString str = url.toString();

    if (str == "about:blank")
        return;

    ui->dirWebViewUrlLineEdit->setText(str);
}

void DirectoryWidget::partsWebView_urlChanged(const QUrl &url)
{
    ui->partsIndexBackButton->setEnabled(ui->partsWebView->history()->canGoBack());
    ui->partsIndexForwardButton->setEnabled(ui->partsWebView->history()->canGoForward());
    ui->partsIndexEditButton->setEnabled(url.scheme() == "file");
    QString str = url.toString();
    ui->partsIndexUrlLineEdit->setText(str);
}

void DirectoryWidget::watchedAutoIndexDirectoryChanged(const QString &path)
{
    watchAutoIndexDirectory(m_currentRootPath);
    updateIndexMenus();

    if (path != m_currentRootPath)
        reloadPartsIndex();

    if (path == m_currentRootPath && !ui->dirWebView->isAutoIndexPage())
        return;

    reloadDirectoryIndex();
}

void DirectoryWidget::cachedDirectoryChanged(const QString &path)
{
    if (path == m_currentRootPath && ui->dirWebView->isAutoIndexPage())
        reloadDirectoryIndex();
}

void DirectoryWidget::deleteSelectedParts()
{
    PartsDeleteDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        emit prepareFileOperation();
        ui->partsTreeView->deleteParts();
        emit fileOperationFinished();
    }
}

void DirectoryWidget::adjustThumbColumnWidth(int width)
{
    ui->partsTreeView->setColumnWidth(1, width);
    Settings::get()->GUIThumbWidth = width;
    ui->partsTreeView->settingsChanged();
}

void DirectoryWidget::previewInProductView(const QFileInfo &fi)
{
    FileMetadata f(fi);
    m_productView->setFile(&f);
    // keep focus on the main window - keyboard handling
    activateWindow();
}

void DirectoryWidget::moveSelectedParts()
{
    if( QMessageBox::question(this,
                              tr("Do you really want to move selected parts?"),
                              tr("Do you really want to move selected parts?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            ==  QMessageBox::Yes)
    {
        emit prepareFileOperation();
        ui->partsTreeView->moveParts();
        emit fileOperationFinished();
    }
}

void DirectoryWidget::setFiltersDialog()
{
    FiltersDialog dlg(m_currentRootPath, this);

    if (dlg.exec())
    {
        emit changeSettings();
    }
}
