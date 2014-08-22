#include <QDir>
#include <QWebHistory>
#include <QtDebug>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>

#include "servertabwidget.h"
#include "ui_servertabwidget.h"
#include "filemodel.h"
#include "filefiltermodel.h"
#include "settings.h"
#include "filtersdialog.h"
#include "extensions/productview/productview.h"


ServerTabWidget::ServerTabWidget(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::ServerTabWidget)
{
	ui->setupUi(this);

	m_productView = new ProductView(this);

    connect(ui->refreshButton, SIGNAL(clicked()),
            MetadataCache::get(), SLOT(clear()));

	ui->techSpecBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	ui->techSpecForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
	ui->techSpecReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
	ui->techSpecGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

	connect(ui->techSpecBackButton, SIGNAL(clicked()),
	        ui->techSpec, SLOT(back()));
	connect(ui->techSpecForwardButton, SIGNAL(clicked()),
	        ui->techSpec, SLOT(forward()));
	connect(ui->techSpecReloadButton, SIGNAL(clicked()),
	        ui->techSpec, SLOT(reload()));
	connect(ui->techSpecUrlLineEdit, SIGNAL(returnPressed()),
	        this, SLOT(techSpecUrlLineEdit_returnPressed()));
	connect(ui->techSpecGoButton, SIGNAL(clicked()),
	        this, SLOT(techSpecGoButton_clicked()));
	connect(ui->techSpecPinButton, SIGNAL(clicked()),
	        this, SLOT(techSpecPinButton_clicked()));
	connect(ui->techSpec, SIGNAL(urlChanged(QUrl)),
	        this, SLOT(techSpec_urlChanged(QUrl)));

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
	connect(ui->partsWebView, SIGNAL(urlChanged(QUrl)),
	        this, SLOT(partsWebView_urlChanged(QUrl)));

    connect(ui->copyToWorkingDirButton, SIGNAL(clicked()),
            ui->partsTreeView, SLOT(copyToWorkingDir()));
	connect(ui->btnDelete, SIGNAL(clicked()),
	        this, SLOT(deleteSelectedParts()));
	connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)),
	        this, SLOT(adjustThumbColumnWidth(int)));

	connect(ui->partsTreeView, SIGNAL(activated(QModelIndex)),
	        this, SLOT(previewInProductView(QModelIndex)));
	connect(ui->partsTreeView, SIGNAL(clicked(QModelIndex)),
	        this, SLOT(previewInProductView(QModelIndex)));
	connect(ui->partsTreeView, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(partsTreeView_doubleClicked(QModelIndex)));

	connect(ui->filterButton, SIGNAL(clicked()),
	        this, SLOT(setFiltersDialog()));
}

ServerTabWidget::~ServerTabWidget()
{
	delete ui;
}

void ServerTabWidget::setDirectory(const QString &rootPath)
{
    setEnabled(false);

    // set the directory to the file model
    ui->partsTreeView->setDirectory(rootPath);
    // handle the ui->partsWebView, custom index-parts*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->partsWebView, "index-parts", true);
    // handle the ui->partsWebView, custom index*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->techSpec, "index", false);

    setEnabled(true);
}

void ServerTabWidget::loadIndexHtml(const QString &rootPath, QWebView *webView, const QString &filterBase, bool hideIfNotFound)
{
    QStringList filters;
    filters << filterBase + "_??.html"
            << filterBase + "_??.htm"
            << filterBase + ".html"
            << filterBase + ".htm";

    QDir dir(rootPath + "/" + TECHSPEC_DIR);
    QStringList indexes = dir.entryList(filters, QDir::Files | QDir::Readable);

    if (indexes.isEmpty())
    {
        webView->setHtml("");
        if (hideIfNotFound) webView->hide();
        return;
    }

    QString selectedIndex = indexes.first();
    indexes.removeFirst();

    foreach(QString index, indexes)
    {
        QString prefix = index.section('.', 0, 0);
        if(prefix.lastIndexOf('_') == prefix.count()-3
                && prefix.right(2) == Settings::get()->getCurrentLanguageCode().left(2))
        {
            selectedIndex = index;
        }
    }

    webView->show();
    webView->load(QUrl::fromLocalFile(dir.path() + "/" + selectedIndex));
}

void ServerTabWidget::changeEvent(QEvent *e)
{
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		if (ui->techSpec->url().path().startsWith("/data/zima-cad-parts") )
			ui->techSpec->loadAboutPage();
		break;
	default:
		break;
	}
}

void ServerTabWidget::settingsChanged()
{
	ui->techSpecDeveloperWidget->setVisible(Settings::get()->DeveloperEnabled
	                                        && Settings::get()->DeveloperTechSpecToolBar);
	ui->partsIndexDeveloperWidget->setVisible(Settings::get()->DeveloperEnabled);
	ui->techSpec->loadAboutPage();
	ui->thumbnailSizeSlider->setValue(Settings::get()->GUIThumbWidth);

    ui->partsTreeView->settingsChanged();
}

void ServerTabWidget::techSpecUrlLineEdit_returnPressed()
{
	techSpecGoButton_clicked();
}

void ServerTabWidget::techSpecGoButton_clicked()
{
	QString str = ui->techSpecUrlLineEdit->text();

	if (str == "ZIMA-CAD-Parts:about")
	{
		ui->techSpec->loadAboutPage();
	}
	else
		ui->techSpec->setUrl(QUrl(str));
}

void ServerTabWidget::partsIndexUrlLineEdit_returnPressed()
{
	partsIndexGoButton_clicked();
}

void ServerTabWidget::partsIndexGoButton_clicked()
{
	QString str = ui->partsIndexUrlLineEdit->text();

	ui->partsWebView->load(QUrl(str));

	if (ui->partsWebView->isHidden())
		ui->partsWebView->show();
}

void ServerTabWidget::partsIndexPinButton_clicked()
{
    ui->partsTreeView->createIndexHtmlFile(ui->partsIndexUrlLineEdit->text(), "index-parts");
}

void ServerTabWidget::techSpecPinButton_clicked()
{
    ui->partsTreeView->createIndexHtmlFile(ui->techSpecUrlLineEdit->text(), "index");
}

void ServerTabWidget::techSpec_urlChanged(const QUrl &url)
{
	ui->techSpecBackButton->setEnabled(ui->techSpec->history()->canGoBack());
	ui->techSpecForwardButton->setEnabled(ui->techSpec->history()->canGoForward());

	QString str = url.toString();

	if (str == "about:blank")
		return;

	ui->techSpecUrlLineEdit->setText(str);
}

void ServerTabWidget::partsWebView_urlChanged(const QUrl &url)
{
	ui->partsIndexBackButton->setEnabled(ui->partsWebView->history()->canGoBack());
	ui->partsIndexForwardButton->setEnabled(ui->partsWebView->history()->canGoForward());
	QString str = url.toString();
	ui->partsIndexUrlLineEdit->setText(str);
}

void ServerTabWidget::deleteSelectedParts()
{
	if( QMessageBox::question(this,
	                          tr("Do you really want to delete selected parts?"),
	                          tr("Do you really want to delete selected parts? This action is irreversible."),
	                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
	        ==  QMessageBox::Yes)
	{
        ui->partsTreeView->deleteParts();
	}
}

void ServerTabWidget::adjustThumbColumnWidth(int width)
{
	ui->partsTreeView->setColumnWidth(1, width);
    Settings::get()->GUIThumbWidth = width;
    ui->partsTreeView->settingsChanged();
}

void ServerTabWidget::previewInProductView(const QModelIndex &index)
{
    QFileInfo fi(ui->partsTreeView->fileInfo(index));
    FileMetadata f(fi);

    if (!m_productView->canHandle(f.type))
	{
		m_productView->hide();
		return;
	}

    m_productView->setFile(&f);
	m_productView->show();
	// keep focus on the main window - keyboard handling
	activateWindow();
}

void ServerTabWidget::partsTreeView_doubleClicked(const QModelIndex &index)
{
    QFileInfo fi(ui->partsTreeView->fileInfo(index));
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
        qDebug() << "Starting ProE:" << exe << f.fileInfo.absoluteFilePath() << "; working dir" << Settings::get()->WorkingDir;
        bool ret = QProcess::startDetached(exe, QStringList() << f.fileInfo.absoluteFilePath(), Settings::get()->WorkingDir);
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

void ServerTabWidget::setFiltersDialog()
{
	FiltersDialog dlg;

	if (dlg.exec())
	{
		emit changeSettings();
	}
}
