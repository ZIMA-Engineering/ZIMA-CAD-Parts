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
#include "errordialog.h"
#include "settings.h"
#include "filtersdialog.h"
#include "extensions/productview/productview.h"


ServerTabWidget::ServerTabWidget(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::ServerTabWidget)
{
	ui->setupUi(this);

	m_productView = new ProductView(this);

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

    m_proxyFileModel = new FileFilterModel(this);

    m_fileModel = new FileModel(this);
    m_proxyFileModel->setSourceModel(m_fileModel);
    ui->partsTreeView->setModel(m_proxyFileModel);

#warning todo
#if 0
	connect(m_serversModel, SIGNAL(filesDeleted()),
	        this, SLOT(filesDeleted()));
	connect(m_serversModel, SIGNAL(partsIndexAlreadyExists(Item*)),
	        this, SLOT(partsIndexOverwrite(Item*)));

    connect(ui->copyToWorkingDirButton, SIGNAL(clicked()),
            m_serversModel, SLOT(copyToWorkingDir()));
#endif
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

#warning "TODO/FIXME: icon for the button to save space"
	connect(ui->filterButton, SIGNAL(clicked()),
	        this, SLOT(setFiltersDialog()));
}

ServerTabWidget::~ServerTabWidget()
{
	delete ui;
}

void ServerTabWidget::setDirectory(const QString &rootPath)
{
    qDebug() << "_______________________________________________ set dir :" << rootPath;
    // set the directory to the file model
    m_fileModel->setDirectory(rootPath);
    int columnCnt = ui->partsTreeView->model()->columnCount(QModelIndex());
    for (int i = 0; i < columnCnt; i++)
        ui->partsTreeView->resizeColumnToContents(i);

    // handle the ui->partsWebView, custom index-parts*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->partsWebView, "index-parts", true);
    // handle the ui->partsWebView, custom index*.html page in "parts" tab
    loadIndexHtml(rootPath, ui->techSpec, "index", false);
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
	ui->techSpec->setDownloadDirectory(Settings::get()->WorkingDir);

    m_fileModel->settingsChanged();

	m_proxyFileModel->setFilterRegExp(Settings::get()->filtersRegex);
	m_proxyFileModel->setShowProeVersions(Settings::get()->ShowProeVersions);

	ui->thumbnailSizeSlider->setValue(Settings::get()->GUIThumbWidth);
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
    m_fileModel->createIndexHtmlFile(ui->partsIndexUrlLineEdit->text(), "index-parts");
}

void ServerTabWidget::techSpecPinButton_clicked()
{
    m_fileModel->createIndexHtmlFile(ui->techSpecUrlLineEdit->text(), "index");


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

void ServerTabWidget::filesDeleted()
{
#warning todo
#if 0
	if (m_serversModel->hasErrors(BaseDataSource::Delete))
	{
		ErrorDialog dlg;
		dlg.setError(tr("Unable to delete files:"));

		QString str = "<html><body><dl>";
		foreach(BaseDataSource::Error *e, m_serversModel->fileErrors(BaseDataSource::Delete))
		{
			str += "<dt>" + e->file->path + ":</dt>";
			str += "<dd>" + e->error + "</dd>";
		}
		str += "</dl></body></html>";

		dlg.setText(str);
		dlg.exec();
	} else {
		// FIXME: if the deletion should occur in another thread and take more time, it will
		// be neccessary to check if the reset is needed (user might be viewing something entirely
		// different by the time it's finished).

		ui->partsTreeView->reset();
	}
#endif
}

void ServerTabWidget::deleteSelectedParts()
{
	if( QMessageBox::question(this,
	                          tr("Do you really want to delete selected parts?"),
	                          tr("Do you really want to delete selected parts? This action is irreversible."),
	                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
	        ==  QMessageBox::Yes)
	{
#warning todo
#if 0
		m_serversModel->deleteFiles(); //TODO/FIXME: maps
		m_serversModel->uncheckAll(); //TODO/FIXME: maps
#endif
	}
}

void ServerTabWidget::adjustThumbColumnWidth(int width)
{
	ui->partsTreeView->setColumnWidth(1, width);
    Settings::get()->GUIThumbWidth = width;
    m_fileModel->settingsChanged();
}

void ServerTabWidget::previewInProductView(const QModelIndex &index)
{
	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->partsTreeView->model())->mapToSource(index);
    QFileInfo fi(m_fileModel->fileInfo(srcIndex));
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
	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->partsTreeView->model())->mapToSource(index);

    QFileInfo fi = m_fileModel->fileInfo(srcIndex);
    FileMetadata f(fi);

    switch (f.type)
	{
	case File::PRT_PROE:
	case File::ASM:
	case File::DRW:
	case File::FRM:
	case File::NEU_PROE:
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
