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


ServerTabWidget::ServerTabWidget(ServersModel *serversModel, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ServerTabWidget),
	m_serversModel(serversModel),
	lastPartsIndexItem(0)

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

    m_fileModel = new FileModel(this);
    m_fileModel->setRootIndex(m_serversModel->rootItem());
    viewHidePartsIndex(m_serversModel->rootItem());

	m_proxyFileModel = new FileFilterModel(this);
	m_proxyFileModel->setSourceModel(m_fileModel);
	ui->partsTreeView->setModel(m_proxyFileModel);

	connect(m_fileModel, SIGNAL(requestColumnResize()),
	        this, SLOT(fileModel_requestColumnResize()));

	connect(m_serversModel, SIGNAL(filesDeleted()),
	        this, SLOT(filesDeleted()));
	connect(m_serversModel, SIGNAL(itemLoaded(const QModelIndex&)),
	        this, SLOT(partsIndexLoaded(const QModelIndex&)));
	connect(m_serversModel, SIGNAL(techSpecAvailable(QUrl)),
	        this, SLOT(loadTechSpec(QUrl)));
	connect(m_serversModel, SIGNAL(partsIndexAlreadyExists(Item*)),
	        this, SLOT(partsIndexOverwrite(Item*)));

    connect(ui->copyToWorkingDirButton, SIGNAL(clicked()),
            m_serversModel, SLOT(copyToWorkingDir()));
	connect(ui->btnDelete, SIGNAL(clicked()),
	        this, SLOT(deleteSelectedParts()));
	connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)),
	        m_fileModel, SLOT(setThumbWidth(int)));
	connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)),
	        this, SLOT(adjustThumbColumnWidth(int)));

	connect(m_serversModel, SIGNAL(fileDownloaded(File*)),
	        m_productView, SLOT(fileDownloaded(File*)));
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

	m_fileModel->setRootIndex(QModelIndex());
	m_fileModel->setThumbWidth(Settings::get()->GUIThumbWidth);
	m_fileModel->setPreviewWidth(Settings::get()->GUIPreviewWidth);

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

void ServerTabWidget::techSpecPinButton_clicked()
{
	Item *it = m_fileModel->getRootItem();

	if (!it)
		return;

	QString lang = Settings::get()->getCurrentLanguageCode().left(2);
	it->server->assignTechSpecUrlToItem(ui->techSpecUrlLineEdit->text(), it, lang, true);
}

void ServerTabWidget::techSpecsIndexOverwrite(Item *item)
{
	if (QMessageBox::warning(this,
	                         tr("Tech specs index already exists"),
	                         tr("Index %1 already exists, would you like to overwrite it?").arg(item->getLabel()),
	                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
		techSpecPinButton_clicked();
	}
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
	Item *it = m_fileModel->getRootItem();

	if (!it)
		return;

	QString lang = Settings::get()->getCurrentLanguageCode().left(2);
	it->server->assignPartsIndexUrlToItem(ui->partsIndexUrlLineEdit->text(), it, lang, true);
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

void ServerTabWidget::fileModel_requestColumnResize()
{
	int columnCnt = ui->partsTreeView->model()->columnCount(QModelIndex());
	for (int i = 0; i < columnCnt; i++)
		ui->partsTreeView->resizeColumnToContents(i);
}

void ServerTabWidget::filesDeleted()
{
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
}

void ServerTabWidget::setPartsIndex(const QModelIndex &index)
{
	//qDebug() << "Set parts index init:" << index.isValid();
	if (!index.isValid())
		return;
	//qDebug() << "Set parts index" << static_cast<Item*>(index.internalPointer())->name;

	m_fileModel->setRootIndex(index);
	partsIndexLoaded(index);
}

void ServerTabWidget::partsIndexLoaded(const QModelIndex &index)
{
	//qDebug() << "ServerTabWidget::partsIndexLoaded" << index.isValid();
	if (!index.isValid())
		return;

	Item *it = static_cast<Item*>(index.internalPointer());

	if (it == m_fileModel->getRootItem())
	{
		viewHidePartsIndex(it);
	}
}

void ServerTabWidget::viewHidePartsIndex(Item *item)
{
	if(!item && !lastPartsIndexItem)
		return;
	else if(!item)
		item = lastPartsIndexItem;

	QStringList filters;
	filters << "index-parts_??.html" << "index-parts_??.htm" << "index-parts.html" << "index-parts.htm";

	QDir dir(item->server->getTechSpecPathForItem(item));
	QStringList indexes = dir.entryList(filters, QDir::Files | QDir::Readable);

	if(indexes.isEmpty())
	{
		ui->partsWebView->setHtml("");
		ui->partsWebView->hide();
		lastPartsIndex = QUrl();
		lastPartsIndexItem = 0;
		return;
	}

	QString selectedIndex = indexes.first();
	indexes.removeFirst();

	foreach(QString index, indexes)
	{
		QString prefix = index.section('.', 0, 0);

		if(prefix.lastIndexOf('_') == prefix.count()-3 && prefix.right(2) == Settings::get()->getCurrentLanguageCode().left(2))
			selectedIndex = index;
	}

	QUrl partsIndex = QUrl::fromLocalFile(dir.path() + "/" + selectedIndex);
	QDateTime modTime = QFileInfo(dir.path() + "/" + selectedIndex).lastModified();

	if(partsIndex == lastPartsIndex)
	{
		if(modTime > lastPartsIndexModTime)
			lastPartsIndexModTime = modTime;
		else if(ui->partsWebView->isHidden()) {
			ui->partsWebView->show();
			return;
		}
	} else {
		lastPartsIndex = partsIndex;
		lastPartsIndexModTime = modTime;
		lastPartsIndexItem = item;
	}

	if(ui->partsWebView->isHidden())
		ui->partsWebView->show();

	ui->partsWebView->load(partsIndex);
}

void ServerTabWidget::deleteSelectedParts()
{
	if( QMessageBox::question(this,
	                          tr("Do you really want to delete selected parts?"),
	                          tr("Do you really want to delete selected parts? This action is irreversible."),
	                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
	        ==  QMessageBox::Yes)
	{
		m_serversModel->deleteFiles(); //TODO/FIXME: maps
		m_serversModel->uncheckAll(); //TODO/FIXME: maps
	}
}

void ServerTabWidget::adjustThumbColumnWidth(int width)
{
	ui->partsTreeView->setColumnWidth(1, width);
}

void ServerTabWidget::previewInProductView(const QModelIndex &index)
{
	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->partsTreeView->model())->mapToSource(index);

	File *f = m_fileModel->getRootItem()->files.at(srcIndex.row());

	if (!m_productView->canHandle(f->type))
	{
		m_productView->hide();
		return;
	}

#warning TODO/FIXME: simplify it
	m_productView->expectFile(f);
    m_productView->fileDownloaded(f);
	m_productView->show();
	// keep focus on the main window - keyboard handling
	activateWindow();
}

void ServerTabWidget::partsTreeView_doubleClicked(const QModelIndex &index)
{
	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->partsTreeView->model())->mapToSource(index);

	File *f = m_fileModel->getRootItem()->files.at(srcIndex.row());

	switch (f->type)
	{
	case File::PRT_PROE:
	case File::ASM:
	case File::DRW:
	case File::FRM:
	case File::NEU_PROE:
	{
		QString exe = Settings::get()->ProeExecutable;
		qDebug() << "Starting ProE:" << exe << f->path << "; working dir" << Settings::get()->WorkingDir;
		bool ret = QProcess::startDetached(exe, QStringList() << f->path, Settings::get()->WorkingDir);
		if (!ret)
			QMessageBox::information(this, tr("ProE Startup Error"),
			                         tr("An error occured while ProE has been requested to start"),
			                         QMessageBox::Ok);
		break;
	}
	default:
		QDesktopServices::openUrl(QUrl::fromLocalFile(f->path));
	}
}

void ServerTabWidget::loadTechSpec(const QUrl &url)
{
	Item *it = m_serversModel->lastTechSpecRequest();

	if(it)
		ui->techSpec->setRootPath(it->server->pathToDataRoot());

	ui->techSpec->load(url);
}

void ServerTabWidget::partsIndexOverwrite(Item *item)
{
	if (QMessageBox::warning(this,
	                         tr("Parts index already exists"),
	                         tr("Parts index %1 already exists, would you like to overwrite it?").arg(item->getLabel()),
	                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
	{
		return;
	}

	Item *it = m_fileModel->getRootItem();

	if (!it)
		return;

	QString lang = Settings::get()->getCurrentLanguageCode().left(2);
	m_serversModel->dataSource()->assignPartsIndexUrlToItem(ui->partsIndexUrlLineEdit->text(), it, lang, true);
}

void ServerTabWidget::setFiltersDialog()
{
	FiltersDialog dlg;

	if (dlg.exec())
	{
		emit changeSettings();
	}
}
