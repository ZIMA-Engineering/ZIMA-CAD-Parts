#include <QDir>
#include <QWebEngineHistory>
#include <QtDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>

#include "directorywidget.h"
#include "ui_directorywidget.h"
#include "filemodel.h"
#include "filefiltermodel.h"
#include "settings.h"
#include "filtersdialog.h"
#include "partsdeletedialog.h"


DirectoryWidget::DirectoryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectoryWidget)
{
    ui->setupUi(this);

    connect(ui->refreshButton, SIGNAL(clicked()),
            this, SLOT(refreshButton_clicked()));

    ui->dirWebViewBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->dirWebViewForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->dirWebViewReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->dirWebViewGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

    connect(ui->dirWebViewBackButton, SIGNAL(clicked()),
            ui->dirWebView, SLOT(back()));
    connect(ui->dirWebViewForwardButton, SIGNAL(clicked()),
            ui->dirWebView, SLOT(forward()));
    connect(ui->dirWebViewReloadButton, SIGNAL(clicked()),
            ui->dirWebView, SLOT(reload()));
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

    ui->partsIndexBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->partsIndexForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->partsIndexReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->partsIndexGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

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
    setEnabled(false);

    // set the directory to the file model
    ui->partsTreeView->setDirectory(rootPath);
    // handle the ui->partsWebView, custom index-parts*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->partsWebView, "index-parts", true);
    // handle the ui->partsWebView, custom index*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->dirWebView, "index", false);

    setEnabled(true);
}

void DirectoryWidget::updateDirectory(const QString &rootPath)
{
    if (ui->partsTreeView->currentPath().compare(rootPath) == 0)
        ui->partsTreeView->directoryChanged();
}

void DirectoryWidget::openAboutPage()
{
    ui->tabWidget->setCurrentIndex(0); // Switch to web view
    ui->dirWebView->loadAboutPage();
}

void DirectoryWidget::loadIndexHtml(const QString &rootPath, QWebEngineView *webView, const QString &filterBase, bool hideIfNotFound)
{
    QStringList filters;
    filters << filterBase + "_??.html"
            << filterBase + "_??.htm"
            << filterBase + ".html"
            << filterBase + ".htm";

    QDir dir(rootPath + "/" + METADATA_DIR);
    QStringList indexes = dir.entryList(filters, QDir::Files | QDir::Readable);

    if (indexes.isEmpty())
    {
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
        loadIndexHtml(d.absolutePath(), webView, filterBase, hideIfNotFound);
        return;
    }

    QString selectedIndex = indexes.first();
    indexes.removeFirst();

    foreach(QString index, indexes)
    {
        QString prefix = index.section('.', 0, 0);
        if(prefix.lastIndexOf('_') == prefix.length()-3
                && prefix.right(2) == Settings::get()->getCurrentLanguageCode().left(2))
        {
            selectedIndex = index;
        }
    }

    webView->show();
    webView->load(QUrl::fromLocalFile(dir.path() + "/" + selectedIndex));
}

void DirectoryWidget::editIndexFile(const QString &path)
{
    QUrl url(path);
    QString editor = Settings::get()->TextEditorPath;

    if (editor.isEmpty()) {
        QDesktopServices::openUrl(url);
        return;
    }

    QStringList args;
    args << url.path();
    QProcess::startDetached(editor, args);
}

void DirectoryWidget::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
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
}

void DirectoryWidget::partsIndexEditButton_clicked()
{
    editIndexFile(ui->partsIndexUrlLineEdit->text());
}

void DirectoryWidget::dirWebViewPinButton_clicked()
{
    ui->partsTreeView->createIndexHtmlFile(ui->dirWebViewUrlLineEdit->text(), "index");
}

void DirectoryWidget::dirWebViewEditButton_clicked()
{
    editIndexFile(ui->dirWebViewUrlLineEdit->text());
}

void DirectoryWidget::refreshButton_clicked()
{
    ui->partsTreeView->refreshRequested();
}

void DirectoryWidget::dirWebView_urlChanged(const QUrl &url)
{
    ui->dirWebViewBackButton->setEnabled(ui->dirWebView->history()->canGoBack());
    ui->dirWebViewForwardButton->setEnabled(ui->dirWebView->history()->canGoForward());
    ui->dirWebViewEditButton->setEnabled(url.scheme() == "file");

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

void DirectoryWidget::deleteSelectedParts()
{
    PartsDeleteDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        ui->partsTreeView->deleteParts();
    }
}

void DirectoryWidget::adjustThumbColumnWidth(int width)
{
    ui->partsTreeView->setColumnWidth(1, width);
    Settings::get()->GUIThumbWidth = width;
    ui->partsTreeView->settingsChanged();
}

void DirectoryWidget::moveSelectedParts()
{
    if( QMessageBox::question(this,
                              tr("Do you really want to move selected parts?"),
                              tr("Do you really want to move selected parts?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            ==  QMessageBox::Yes)
    {
        ui->partsTreeView->moveParts();
    }
}

void DirectoryWidget::setFiltersDialog()
{
    FiltersDialog dlg;

    if (dlg.exec())
    {
        emit changeSettings();
    }
}
