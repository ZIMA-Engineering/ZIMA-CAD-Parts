/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QLocale>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include <QRegExp>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDesktopServices>
#include <QWebHistory>
#include <QProcess>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversmodel.h"
#include "downloadmodel.h"
#include "filtersdialog.h"
#include "item.h"
#include "zimautils.h"
#include "errordialog.h"
#include "filefilters/extensionfilter.h"
#include "filefilters/versionfilter.h"

QList<FilterGroup> MainWindow::filterGroups;
QSettings * MainWindow::settings;
QString MainWindow::currentMetadataLang;

MainWindow::MainWindow(QTranslator *translator, QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindowClass),
	  translator(translator),
	  historyCurrentIndex(-1),
	  historySize(0),
	  techSpecToolBar(0),
	  lastPartsIndexItem(0),
	  productView(0)
{
	downloading = false;

	qApp->setWindowIcon(QIcon(":/gfx/icon.png"));

	settings = new QSettings(this);
	bool useSplash = settings->value("GUI/Splash/Enabled", true).toBool();
	QSplashScreen *splash;

	if( useSplash )
	{
		QPixmap pixmap(":/gfx/splash.png");

		splash = new QSplashScreen(pixmap);
		splash->setMask(pixmap.mask());
		splash->show();
	}

	ui->setupUi(this);

	ui->dirTreePathGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

	ui->treeBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	ui->treeForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));

	connect(ui->treeBackButton, SIGNAL(clicked()), this, SLOT(historyBack()));
	connect(ui->treeForwardButton, SIGNAL(clicked()), this, SLOT(historyForward()));

	connect(ui->goHomeDirButton, SIGNAL(clicked()), this, SLOT(goToWorkingDirectory()));
	connect(ui->dirTreePathLineEdit, SIGNAL(returnPressed()), this, SLOT(descentTo()));
	connect(ui->dirTreePathGoButton, SIGNAL(clicked()), this, SLOT(descentTo()));
	connect(ui->dirTreePathOpenButton, SIGNAL(clicked()), this, SLOT(openDirTreePath()));

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

	setupDeveloperMode();

	servers = loadDataSources();
	loadSettings();

	restoreState(settings->value("state", QByteArray()).toByteArray());
	restoreGeometry(settings->value("geometry", QByteArray()).toByteArray());

	connect(ui->btnDownload, SIGNAL(clicked()), this, SLOT(downloadButton()));
	connect(ui->btnSettings, SIGNAL(clicked()), this, SLOT(showSettings()));
	connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(setWorkingDirectoryDialog()));
	connect(ui->openWorkDirButton, SIGNAL(clicked()), this, SLOT(openWorkingDirectory()));
	connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
	connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteSelectedParts()));
	connect(ui->startStopDownloadBtn, SIGNAL(clicked()), this, SLOT(toggleDownload()));

	ui->thumbnailSizeSlider->setValue(settings->value("GUI/ThumbWidth", 32).toInt());

	//connect(ui->treeLeft, SIGNAL(expanded(QModelIndex)), ui->treeLeft, SIGNAL(clicked(QModelIndex)));

	//connect(ui->filtersListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(rebuildFilters()));
	connect(ui->filtersButton, SIGNAL(clicked()), this, SLOT(setFiltersDialog()));

	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);

	ServersModel *sm = new ServersModel(this);
	sm->setServerData(servers);
	sm->retranslateMetadata();
	ui->treeLeft->setModel(sm);
	ui->treeLeft->setContextMenuPolicy(Qt::CustomContextMenu);

	dirTreeSignalMapper = new QSignalMapper(this);

	connect(dirTreeSignalMapper, SIGNAL(mapped(int)), this, SLOT(spawnZimaUtilityOnDir(int)));

	connect(sm, SIGNAL(loadingItem(Item*)), this, SLOT(loadingItem(Item*)));
	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), this, SLOT(itemLoaded(const QModelIndex&)));
	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), this, SLOT(partsIndexLoaded(const QModelIndex&)));
	connect(sm, SIGNAL(allItemsLoaded()), this, SLOT(allItemsLoaded()));
	connect(sm, SIGNAL(techSpecAvailable(QUrl)), this, SLOT(loadTechSpec(QUrl)));
	connect(sm, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));
	connect(sm, SIGNAL(autoDescentProgress(QModelIndex)), this, SLOT(autoDescentProgress(QModelIndex)));
	connect(sm, SIGNAL(autoDescentCompleted(QModelIndex)), this, SLOT(autoDescendComplete(QModelIndex)));
	connect(sm, SIGNAL(autoDescentNotFound()), this, SLOT(autoDescentNotFound()));
	connect(sm, SIGNAL(techSpecsIndexAlreadyExists(Item*)), this, SLOT(techSpecsIndexOverwrite(Item*)));
	connect(sm, SIGNAL(partsIndexAlreadyExists(Item*)), this, SLOT(partsIndexOverwrite(Item*)));

	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), sm, SLOT(requestTechSpecs(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(activated(const QModelIndex&)), sm, SLOT(requestTechSpecs(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), this, SLOT(trackHistory(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(activated(const QModelIndex&)), this, SLOT(trackHistory(const QModelIndex&)));
//	connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), sm, SLOT(requestTechSpecs(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(dirTreeContextMenu(QPoint)));

	fm = new FileModel(this);

	fm->setThumbWidth( settings->value("GUI/ThumbWidth", 32).toInt() );
	fm->setPreviewWidth( settings->value("GUI/PreviewWidth", 256).toInt() );

	connect(fm, SIGNAL(requestColumnResize()), this, SLOT(treeExpandedOrCollaped()));
	connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)), fm, SLOT(setThumbWidth(int)));
	connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(adjustThumbColumnWidth(int)));

	proxy = new FileFilterModel(this);
	proxy->setSourceModel(fm);

	filterGroups << FilterGroup("ProE", "Pro/Engineer");
	filterGroups.last()
	        << new ExtensionFilter(File::PRT_PROE)
	        << new ExtensionFilter(File::ASM)
	        << new ExtensionFilter(File::DRW)
	        << new ExtensionFilter(File::FRM)
	        << new ExtensionFilter(File::NEU_PROE)
	        << new VersionFilter();

	filterGroups << FilterGroup("CATIA", "CATIA");
	filterGroups.last()
	        << new ExtensionFilter(File::CATPART)
	        << new ExtensionFilter(File::CATPRODUCT)
	        << new ExtensionFilter(File::CATDRAWING);

	filterGroups << FilterGroup("NX", "NX (UGS)");
	filterGroups.last()
	        << new ExtensionFilter(File::PRT_NX);

	filterGroups << FilterGroup("SolidWorks", "SolidWorks");
	filterGroups.last().filters
	        << new ExtensionFilter(File::SLDPRT)
	        << new ExtensionFilter(File::SLDASM)
	        << new ExtensionFilter(File::SLDDRW);

	filterGroups << FilterGroup("SolidEdge", "Solid Edge");
	filterGroups.last()
	        << new ExtensionFilter(File::PAR)
	        << new ExtensionFilter(File::PSM)
	        << new ExtensionFilter(File::ASM)
	        << new ExtensionFilter(File::DFT);

	filterGroups << FilterGroup("Invertor", "INVERTOR");
	filterGroups.last()
	        << new ExtensionFilter(File::IPT)
	        << new ExtensionFilter(File::IAM)
	        << new ExtensionFilter(File::IDW);

	filterGroups << FilterGroup("CADNeutral", "CAD NEUTRAL");
	filterGroups.last()
	        << new ExtensionFilter(File::STEP)
	        << new ExtensionFilter(File::IGES)
	        << new ExtensionFilter(File::DWG)
	        << new ExtensionFilter(File::DXF);

	filterGroups << FilterGroup("NonCAD", "NonCAD");
	filterGroups.last()
	        << new ExtensionFilter(File::STL)
	        << new ExtensionFilter(File::BLEND)
	        << new ExtensionFilter(File::PDF);

	loadFilters();
	rebuildFilters();

	QList<int> partsSize;
	partsSize << (int)(ui->tab->height()*0.40) << (int)(ui->tab->height()*0.60);
	ui->partsSplitter->setSizes(partsSize);

	ui->tree->setModel(proxy);

	//connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
//	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
//	connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), this, SLOT(setPartsIndex(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(activated(const QModelIndex&)), this, SLOT(setPartsIndex(const QModelIndex&)));
//	connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), this, SLOT(setPartsIndex(const QModelIndex&)));
	connect(sm, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
	connect(sm, SIGNAL(filesDownloaded()), this, SLOT(filesDownloaded()));
	connect(sm, SIGNAL(filesDeleted()), this, SLOT(filesDeleted()));

	downloadModel = new DownloadModel(this);

	connect(ui->deleteQueueBtn, SIGNAL(clicked()), downloadModel, SLOT(clear()));

	downloadModel->registerHandler(DownloadModel::ServersModel, sm);
	downloadModel->registerHandler(DownloadModel::TechSpec, ui->techSpec);

	ui->downloadTreeView->setModel(downloadModel);
	ui->downloadTreeView->setItemDelegate(new DownloadDelegate(this));

	sm->setDownloadQueue(downloadModel);

	if( sm->loadQueue(settings) )
		ui->startStopDownloadBtn->setText(tr("Resume"));

	currentServer = 0;

	langs << "en_US" << "cs_CZ" << "de_DE" << "ru_RU";

	int cnt = langs.count();
	QString currentLang = getCurrentLanguageCode().left(2);

	langButtonGroup = new QButtonGroup(this);
	langButtonGroup->setExclusive(true);

	for(int i = 0; i < cnt; i++)
	{
		QString lang = langs[i].left(2);

		QPushButton *flag = new QPushButton(QIcon(QString(":/gfx/flags/%1.png").arg(lang)), "", this);
		flag->setFlat(true);
		flag->setCheckable(true);
		flag->setStyleSheet("width: 16px; height: 16px; margin: 0; padding: 1px;");

		if(currentLang == lang)
			flag->setChecked(true);

		langButtonGroup->addButton(flag, i);

		ui->langsHorizontalLayout->addWidget(flag);
	}

	connect(langButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeLanguage(int)));

	// History
	ui->treeBackButton->setEnabled(false);
	ui->treeForwardButton->setEnabled(false);

#ifdef Q_OS_MAC
	QMenu *menu = new QMenu(this);
	QAction *settingsAction = new QAction(tr("Preferences"), this);
	settingsAction->setMenuRole(QAction::PreferencesRole);
	connect(settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));

	menu->addAction(settingsAction);
	menuBar()->addMenu(menu);
#endif

	// Make shortcut Ctrl+C or Cmd+C available
	QAction *copyAction = ui->techSpec->pageAction(QWebPage::Copy);
	copyAction->setShortcut(QKeySequence::Copy);
	ui->techSpec->addAction(copyAction);

	ui->techSpec->setDownloadQueue(downloadModel);

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

	goToWorkingDirectory();

	QAction *act = new QAction(this);
	act->setShortcut(QKeySequence("Ctrl+Q"));

	connect(act, SIGNAL(triggered()), qApp, SLOT(quit()));

	addAction(act);

	act = new QAction(this);
	act->setShortcut(QKeySequence("Ctrl+L"));

	connect(act, SIGNAL(triggered()), this, SLOT(selectDirTreePath()));

	addAction(act);

	showOrHideProductView();

	if( useSplash )
	{
		SleeperThread::msleep( settings->value("GUI/Splash/Duration", 1500).toInt() );

		splash->finish(this);
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::loadExtensions()
{
//	for(int i = 0; i < EXTENSIONS_COUNT; i++)
//	{
//		switch(i)
//		{
//		case PTC_PRODUCT_VIEWER:
//			break;
//		default:
//			continue;
//		}
//	}
}

void MainWindow::setupDeveloperMode()
{
	bool developer = settings->value("Developer/Enabled", false).toBool();
	bool techSpecToolBarEnabled = developer && settings->value("Developer/TechSpecToolBar", true).toBool();

	// Tech spec toolbar
	if(techSpecToolBarEnabled && !techSpecToolBar)
	{
		techSpecToolBar = new QToolBar(this);

		techSpecBackAction = techSpecToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowLeft), tr("Back"), ui->techSpec, SLOT(back()));
		techSpecForwardAction = techSpecToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowRight), tr("Forward"), ui->techSpec, SLOT(forward()));
		techSpecToolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Reload"), ui->techSpec, SLOT(reload()));

		urlBar = new QLineEdit(this);

		connect(urlBar, SIGNAL(returnPressed()), this, SLOT(goToUrl()));

		techSpecToolBar->addWidget(urlBar);
		techSpecToolBar->addAction(style()->standardIcon(QStyle::SP_CommandLink), tr("Go"), this, SLOT(goToUrl()));

		connect(ui->techSpec, SIGNAL(urlChanged(QUrl)), this, SLOT(updateUrlBar(QUrl)));

		techSpecToolBar->addAction(QIcon(":/gfx/pin.png"), tr("Pin this URL to current directory in tree (write permission required)"), this, SLOT(assignUrlToDirectory()));

		techSpecToolBar->setIconSize(QSize(20, 20));

		static_cast<QVBoxLayout*>(ui->tabWidget->widget(TECH_SPECS)->layout())->insertWidget(0, techSpecToolBar);

		// Parts web view toolbar
		partsIndexToolBar = new QToolBar(this);
		partsIndexUrlBar = new QLineEdit(this);

		partsIndexToolBar->setIconSize(QSize(20, 20));

		connect(partsIndexUrlBar, SIGNAL(returnPressed()), this, SLOT(goToPartsIndexUrl()));

		partsIndexToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowLeft), tr("Back"), ui->partsWebView, SLOT(back()));
		partsIndexToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowRight), tr("Forward"), ui->partsWebView, SLOT(forward()));
		partsIndexToolBar->addWidget(partsIndexUrlBar);
		partsIndexToolBar->addAction(style()->standardIcon(QStyle::SP_CommandLink), tr("Go"), this, SLOT(goToPartsIndexUrl()));

		connect(ui->partsWebView, SIGNAL(urlChanged(QUrl)), this, SLOT(updatePartsUrlBar(QUrl)));

		partsIndexToolBar->addAction(QIcon(":/gfx/pin.png"), tr("Pin this URL to current parts index (write permission required)"), this, SLOT(assignPartsIndexUrlToDirectory()));

		static_cast<QVBoxLayout*>(ui->tabWidget->widget(PARTS)->layout())->insertWidget(0, partsIndexToolBar);
	} else if(!techSpecToolBarEnabled && techSpecToolBar) {
		techSpecToolBar->deleteLater();
		urlBar->deleteLater();

		partsIndexToolBar->deleteLater();
		partsIndexUrlBar->deleteLater();

		techSpecToolBar = 0;

		disconnect(ui->techSpec, SIGNAL(urlChanged(QUrl)), this, SLOT(updateUrlBar(QUrl)));
		disconnect(ui->partsWebView, SIGNAL(urlChanged(QUrl)), this, SLOT(updatePartsUrlBar(QUrl)));
	}
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);

		if( ui->techSpec->url().path().startsWith("/data/zima-cad-parts") )
			ui->techSpec->loadAboutPage();
	} else QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	settings->setValue("state", saveState());
	settings->setValue("geometry", saveGeometry());

	//	int cnt = ui->filtersListWidget->count();

	//	settings->beginGroup("PartFilters");
	//	for(int i = 0; i < cnt; i++)
	//		settings->setValue( QString::number(i), ui->filtersListWidget->item(i)->checkState() == Qt::Checked );
	//	settings->endGroup();

	static_cast<ServersModel*>(ui->treeLeft->model())->saveQueue(settings);

	qDeleteAll(servers);

	QMainWindow::closeEvent(e);
}

void MainWindow::downloadButton()
{
	ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());
	sm->downloadFiles( ui->editDir->text() );

	static_cast<ServersModel*>( ui->treeLeft->model() )->uncheckAll();
	ui->tabWidget->setCurrentIndex(MainWindow::DOWNLOADS);

	//int columnCnt = ui->downloadTreeView->model()->columnCount(QModelIndex());

	ui->downloadTreeView->resizeColumnToContents(0);
	ui->downloadTreeView->resizeColumnToContents(2);

	downloading = true;
}

void MainWindow::setWorkingDirectoryDialog()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("ZIMA-CAD-Parts - set working directory"), ui->editDir->text());
	if (!str.isEmpty())
	{
		settings->setValue("HomeDir", "");
		settings->setValue("WorkingDir", str);

		ui->techSpec->setDownloadDirectory(str);
		ui->editDir->setText(str);
	}
}

void MainWindow::showSettings(SettingsDialog::Section section)
{
	SettingsDialog *settingsDlg = new SettingsDialog(settings, loadDataSources(), &translator, this);
	settingsDlg->setSection(section);

	//settingsDlg->loadSettings(settings);
	int result = settingsDlg->exec();

	if (result == QDialog::Accepted)
	{
		settingsDlg->saveSettings();
		//((PartsModel*)ui->tree->model())->setShowEmpty(settings->value("gui/showEmpty", true).toBool());
		ServersModel* sm = static_cast<ServersModel*>(ui->treeLeft->model());
		fm->setRootIndex(QModelIndex());

		QList<BaseDataSource*> oldServers = servers;

		servers = settingsDlg->getDatasources();
		sm->setServerData(servers);

		qDeleteAll(oldServers);

		saveSettings();

		fm->setThumbWidth( settings->value("GUI/ThumbWidth", 32).toInt() );
		fm->setPreviewWidth( settings->value("GUI/PreviewWidth", 256).toInt() );

		ui->thumbnailSizeSlider->setValue(settings->value("GUI/ThumbWidth", 32).toInt());

		setupDeveloperMode();

		int langIndex = SettingsDialog::langIndex(getCurrentLanguageCode()) - 1;

		langButtonGroup->button(langIndex)->setChecked(true);
		changeLanguage(langIndex);

		// Prune tree history
		history.clear();
		historyCurrentIndex = -1;
		historySize = 0;

		ui->treeBackButton->setEnabled(false);
		ui->treeForwardButton->setEnabled(false);

		loadZimaUtils();

		ui->techSpec->loadAboutPage();

		allItemsLoaded();

		showOrHideProductView();
	}

	delete settingsDlg;
}

void MainWindow::updateClicked()
{
	Item* i = fm->getRootItem();

	if( i == 0)
		return;

	fm->prepareForUpdate();

	ServersModel *sm = static_cast<ServersModel*>( ui->treeLeft->model() );
	sm->refresh(i);
	sm->requestTechSpecs(i);
//	ui->btnUpdate->setEnabled(false);
}

void MainWindow::deleteSelectedParts()
{
	if( QMessageBox::question(this,
	                          tr("Do you really want to delete selected parts?"),
	                          tr("Do you really want to delete selected parts? This action is irreversible."),
	                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
	        ==  QMessageBox::Yes)
	{
		ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());

		sm->deleteFiles();
		sm->uncheckAll();
	}
}

void MainWindow::searchClicked()
{
	QMessageBox::information(this, "ZimaParts", "Not yet implemented");
}

void MainWindow::treeExpandedOrCollaped()
{
	int columnCnt = ui->tree->model()->columnCount(QModelIndex());
	for (int i = 0; i < columnCnt; i++)
		ui->tree->resizeColumnToContents(i);
}

void MainWindow::serverSelected(const QModelIndex &i)
{
	if (i.internalPointer())
	{
		Item* item = static_cast<Item*>(i.internalPointer());

		static_cast<ServersModel*>(ui->treeLeft->model())->loadItem(item);
	}
}

void MainWindow::updateStatus(QString str)
{
	statusDir->setText(str);
}

void MainWindow::loadingItem(Item *item)
{
	statusDir->setText(tr("Loading %1...").arg(item->getLabel()));
}

void MainWindow::itemLoaded(const QModelIndex &index)
{
	Q_UNUSED(index);
//	ui->btnUpdate->setEnabled(true);

//	setPartsIndex(index);
}

void MainWindow::allItemsLoaded()
{
	statusDir->setText(tr("All items loaded."));
}

void MainWindow::loadTechSpec(QUrl url)
{
	Item *it = static_cast<ServersModel*>(ui->treeLeft->model())->lastTechSpecRequest();

	if(it)
		ui->techSpec->setRootPath(it->server->pathToDataRoot());

	ui->techSpec->load(url);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	switch( event->key() )
	{
	case Qt::Key_F5:
		updateClicked();
		break;
	case Qt::Key_Escape:
		//static_cast<ServersModel*>( ui->treeLeft->model() )->abort();
		stopDownload();
		updateStatus(tr("Aborted."));

		break;
	}
}

void MainWindow::errorOccured(QString error)
{
	ui->treeLeft->setEnabled(true);
	ui->btnUpdate->setEnabled(true);

	updateStatus(tr("FTP error."));
	QMessageBox::warning(this, tr("FTP error"), error);
}

void MainWindow::filesDownloaded()
{
	updateStatus(tr("Parts downloaded."));
	downloading = false;
}

void MainWindow::resumeDownload()
{
	downloadModel->resume();
	ui->startStopDownloadBtn->setText(tr("Stop"));
	downloading = true;
}

void MainWindow::stopDownload()
{
	downloadModel->stop();
	ui->startStopDownloadBtn->setText(tr("Resume"));
	downloading = false;
}

void MainWindow::toggleDownload()
{
	if(downloadModel->isEmpty())
		return;

	if( downloadModel->isDownloading() )
		stopDownload();
	else
		resumeDownload();
}

void MainWindow::setFiltersDialog()
{
	FiltersDialog *dlg = new FiltersDialog(this);

	if(dlg->exec() == QDialog::Accepted)
	{
		saveFilters();
		rebuildFilters();
	}
}

void MainWindow::rebuildFilters()
{
	QStringList expressions;

	int cnt = filterGroups.count();

	for(int i = 0; i < cnt; i++)
	{
		if(!filterGroups[i].enabled)
			continue;

		int filterCnt = filterGroups[i].filters.count();

		for(int j = 0; j < filterCnt; j++)
		{
			switch(filterGroups[i].filters[j]->filterType())
			{
			case FileFilter::Extension:
				if(filterGroups[i].filters[j]->enabled)
					expressions << File::getRxForFileType(filterGroups[i].filters[j]->type);
				break;

			case FileFilter::Version:
				proxy->setShowProeVersions(filterGroups[i].filters[j]->enabled);
				break;
			}


		}
	}

	expressions.removeDuplicates();

	QRegExp rx( "^" + expressions.join("|") + "$" );
	proxy->setFilterRegExp(rx);
}

void MainWindow::filesDeleted()
{
	ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());

	if(sm->hasErrors(BaseDataSource::Delete))
	{
		ErrorDialog *dlg = new ErrorDialog(this);

		dlg->setError(tr("Unable to delete files:"));

		QString str = "<html><body><dl>";

		foreach(BaseDataSource::Error *e, sm->fileErrors(BaseDataSource::Delete))
		{
			str += "<dt>" + e->file->path + ":</dt>";
			str += "<dd>" + e->error + "</dd>";
		}

		str += "</dl></body></html>";

		dlg->setText(str);

		dlg->exec();

	} else {
		// FIXME: if the deletion should occur in another thread and take more time, it will
		// be neccessary to check if the reset is needed (user might be viewing something entirely
		// different by the time it's finished).

		ui->tree->reset();
	}
}

void MainWindow::selectDirTreePath()
{
	ui->dirTreePathLineEdit->selectAll();
	ui->dirTreePathLineEdit->setFocus();
}

void MainWindow::openWorkingDirectory()
{
	QString workingDir = ui->editDir->text();

	if(!QFile::exists(workingDir))
	{
		QDir dir;

		if(!dir.mkpath(workingDir))
		{
			QMessageBox::warning(this, tr("Unable to create working directory"), tr("Unable to create working directory: %1").arg(workingDir));
			return;
		}
	}

	QDesktopServices::openUrl(QUrl::fromLocalFile(workingDir));
}

void MainWindow::changeLanguage(int lang)
{
	if(getCurrentMetadataLanguageCode() == langs[lang])
		return;

	currentMetadataLang = langs[lang];

	static_cast<ServersModel*>(ui->treeLeft->model())->retranslateMetadata();

	viewHidePartsIndex();
}

void MainWindow::goToUrl()
{
	QString str = urlBar->text();

	if(urlBar->text() == "ZIMA-CAD-Parts:about")
	{
		ui->techSpec->loadAboutPage();
		return;
	}

	ui->techSpec->setUrl(QUrl(str));
}

void MainWindow::updateUrlBar(QUrl url)
{
	techSpecBackAction->setEnabled(ui->techSpec->history()->canGoBack());
	techSpecForwardAction->setEnabled(ui->techSpec->history()->canGoForward());

	QString str = url.toString();

	if(str == "about:blank")
		return;

	urlBar->setText(str);
}

void MainWindow::goToPartsIndexUrl()
{
	QString str = partsIndexUrlBar->text();

	ui->partsWebView->load(QUrl(str));

	if(ui->partsWebView->isHidden())
		ui->partsWebView->show();
}

void MainWindow::updatePartsUrlBar(QUrl url)
{
	QString str = url.toString();

	partsIndexUrlBar->setText(str);
}

void MainWindow::setPartsIndex(const QModelIndex &index)
{
	qDebug() << "Set parts index" << static_cast<Item*>(index.internalPointer())->name;

	fm->setRootIndex(index);
	partsIndexLoaded(index);
}

void MainWindow::partsIndexLoaded(const QModelIndex &index)
{
	Item *it = static_cast<Item*>(index.internalPointer());

	if(it == fm->getRootItem())
	{
		viewHidePartsIndex(it);

		ui->dirTreePathLineEdit->setText(it->server->name() + it->pathRelativeToDataSource());
	}
}

void MainWindow::viewHidePartsIndex(Item *item)
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

		if(prefix.lastIndexOf('_') == prefix.count()-3 && prefix.right(2) == getCurrentMetadataLanguageCode().left(2))
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

void MainWindow::descentTo()
{
	autoDescentPath = QDir::cleanPath(ui->dirTreePathLineEdit->text()).trimmed();
	static_cast<ServersModel*>(ui->treeLeft->model())->descentTo( autoDescentPath );
}

void MainWindow::autoDescentProgress(const QModelIndex &index)
{
	Item *item = static_cast<Item*>(index.internalPointer());

	qDebug() << "Progress expand" << index << item->name << item->parent->name;

	lastFoundIndex = index;
	ui->treeLeft->expand(index);
}

void MainWindow::autoDescendComplete(const QModelIndex &index)
{
	ui->treeLeft->expand(index);
//	ui->treeLeft->setCurrentIndex(index);
	setPartsIndex(index);
	static_cast<ServersModel*>(ui->treeLeft->model())->requestTechSpecs(index);
	trackHistory(index);
}

void MainWindow::autoDescentNotFound()
{
	if(lastFoundIndex.isValid())
	{
		setPartsIndex(lastFoundIndex);
		static_cast<ServersModel*>(ui->treeLeft->model())->requestTechSpecs(lastFoundIndex);
	}

	QMessageBox::warning(this, tr("Directory not found"), tr("Directory not found: %1").arg(autoDescentPath));
}

void MainWindow::adjustThumbColumnWidth(int width)
{
	ui->tree->setColumnWidth(1, width);
}

void MainWindow::assignUrlToDirectory(bool overwrite)
{
	Item *it = fm->getRootItem();

	if(it)
		static_cast<ServersModel*>(ui->treeLeft->model())->assignTechSpecUrlToItem(urlBar->text(), it, overwrite);
}

void MainWindow::techSpecsIndexOverwrite(Item *item)
{
	Q_UNUSED(item);
	if(QMessageBox::warning(this, tr("Tech specs index already exists"), tr("Index already exists, would you like to overwrite it?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		assignUrlToDirectory(true);
}

void MainWindow::assignPartsIndexUrlToDirectory(bool overwrite)
{
	Item *it = fm->getRootItem();

	if(it)
		static_cast<ServersModel*>(ui->treeLeft->model())->assignPartsIndexUrlToItem(partsIndexUrlBar->text(), it, overwrite);
}

void MainWindow::partsIndexOverwrite(Item *item)
{
	Q_UNUSED(item);
	if(QMessageBox::warning(this, tr("Parts index already exists"), tr("Parts index already exists, would you like to overwrite it?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		assignPartsIndexUrlToDirectory(true);
}

void MainWindow::openDirTreePath()
{
	QString path = ui->dirTreePathLineEdit->text();
	QStringList parts = path.split('/');
	QString dataRoot;

	if(parts.isEmpty() || (dataRoot = static_cast<ServersModel*>(ui->treeLeft->model())->translateDataSourceNameToPath(parts.first())).isEmpty())
	{
		QMessageBox::warning(this, tr("Not found"), tr("Path %1 does not exist.").arg(path));
		return;
	}

	parts.removeFirst();
	QDesktopServices::openUrl(QUrl::fromLocalFile(dataRoot + "/" + parts.join("/")));
}

void MainWindow::loadSettings()
{
	ui->editDir->setText(settings->value("WorkingDir", QDir::homePath() + "/ZIMA-CAD-Parts").toString());
	ui->techSpec->setDownloadDirectory(ui->editDir->text());

	loadZimaUtils();
}

void MainWindow::loadZimaUtils()
{
	zimaUtils.clear();

	settings->beginGroup("ExternalPrograms");

	for(int i = 0; i < ZimaUtils::ZimaUtilsCount; i++)
	{
		settings->beginGroup(ZimaUtils::internalNameForUtility(i));
		zimaUtils << settings->value("Executable").toString();
		settings->endGroup();
	}

	settings->endGroup();
}

QList<BaseDataSource*> MainWindow::loadDataSources()
{
	QString currentLang = getCurrentMetadataLanguageCode().left(2);
	QList<BaseDataSource*> servers;

	settings->beginGroup("DataSources");
	foreach(QString str, settings->childGroups())
	{
		settings->beginGroup(str);

		QString dataSourceType = settings->value("DataSourceType", "ftp").toString();

		if( dataSourceType == "ftp" )
		{
			FtpDataSource *s = new FtpDataSource();
			s->loadSettings(*settings);
			servers.append(s);
		} else if ( dataSourceType == "local" ) {
			LocalDataSource *s = new LocalDataSource();
			s->loadSettings(*settings);
			servers.append(s);
		}

		settings->endGroup();
	}
	settings->endGroup();

	return servers;
}

void MainWindow::saveSettings()
{
	int i = 0;
	QString str;
	settings->remove("DataSources");
	settings->beginGroup("DataSources");
	foreach(BaseDataSource *bs, servers)
	{
		settings->beginGroup(QString::number(i++));
		switch( bs->dataSource )
		{
		case LOCAL: {
			LocalDataSource *s = static_cast<LocalDataSource*>(bs);

			s->saveSettings(*settings);
			break;
		}
		case FTP: {
			FtpDataSource *s = static_cast<FtpDataSource*>(bs);

			s->saveSettings(*settings);
			break;
		}
		default:
			break;
		}

		settings->endGroup();
	}
	settings->endGroup();

	settings->setValue("WorkingDir", ui->editDir->text());
	settings->sync();
}

void MainWindow::loadFilters()
{
	settings->beginGroup("PartFilters");

	int cnt = filterGroups.count();

	for(int i = 0; i < cnt; i++)
	{
		settings->beginGroup(filterGroups[i].internalName);
		filterGroups[i].enabled = settings->value("Enabled", true).toBool();

		int filterCnt = filterGroups[i].filters.count();

		for(int j = 0; j < filterCnt; j++)
			filterGroups[i].filters[j]->load(settings);

		settings->endGroup();
	}

	settings->endGroup();
}

void MainWindow::saveFilters()
{
	settings->beginGroup("PartFilters");

	int cnt = filterGroups.count();

	for(int i = 0; i < cnt; i++)
	{
		settings->beginGroup(filterGroups[i].internalName);
		settings->setValue("Enabled", filterGroups[i].enabled);

		int filterCnt = filterGroups[i].filters.count();

		for(int j = 0; j < filterCnt; j++)
			filterGroups[i].filters[j]->save(settings);

		settings->endGroup();
	}

	settings->endGroup();
}

void MainWindow::dirTreeContextMenu(QPoint point)
{
	QModelIndex i = ui->treeLeft->currentIndex();

	if(!i.isValid())
		return;

	Item *it = static_cast<Item*>(i.internalPointer());

	if(it->server->dataSource != LOCAL)
		return;

	QMenu *menu = new QMenu(this);

	menu->addAction(QIcon(":/gfx/gohome.png"), tr("Set as working directory"), this, SLOT(setWorkingDirectory()));
	menu->addSeparator();

	dirTreeSignalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PTC-Cleaner.png"), "Clean with ZIMA-PTC-Cleaner", dirTreeSignalMapper, SLOT(map())), ZimaUtils::ZimaPtcCleaner);
	dirTreeSignalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-CAD-Sync.png"), "Sync with ZIMA-CAD-Sync", dirTreeSignalMapper, SLOT(map())), ZimaUtils::ZimaCadSync);
	dirTreeSignalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PS2PDF.png"), "Convert postscript to PDF with ZIMA-PS2PDF", dirTreeSignalMapper, SLOT(map())), ZimaUtils::ZimaPs2Pdf);
	dirTreeSignalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-STEP-Edit.png"), "Edit step files with ZIMA-STEP-Edit", dirTreeSignalMapper, SLOT(map())), ZimaUtils::ZimaStepEdit);

	menu->exec(ui->treeLeft->mapToGlobal(point));
	menu->deleteLater();
}

void MainWindow::spawnZimaUtilityOnDir(int i)
{
	QString label = ZimaUtils::labelForUtility(i);

	if(zimaUtils[i].isEmpty())
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Please first configure path to %1 executable.").arg(label));
		showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	QString executable = zimaUtils[i];

	if(!QFile::exists(executable))
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Path '%1' to %2 executable does not exists!").arg(executable).arg(label));
		showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	qDebug() << "Spawn" << label;

	QStringList args;

	args << static_cast<Item*>(ui->treeLeft->currentIndex().internalPointer())->path;

	QProcess::startDetached(executable, args);
}

void MainWindow::setWorkingDirectory()
{
	Item *it = static_cast<Item*>(ui->treeLeft->currentIndex().internalPointer());

	settings->setValue("HomeDir", it->server->name() + it->pathRelativeToDataSource());
	settings->setValue("WorkingDir", it->path);

	ui->techSpec->setDownloadDirectory(it->path);

	ui->editDir->setText(it->path);
}

void MainWindow::goToWorkingDirectory()
{
	autoDescentPath = settings->value("HomeDir").toString();

	if(autoDescentPath.isEmpty())
		return;

	static_cast<ServersModel*>(ui->treeLeft->model())->descentTo(autoDescentPath);
}

void MainWindow::trackHistory(const QModelIndex &index)
{
	if(historyCurrentIndex >= 0 && history[historyCurrentIndex].internalPointer() == index.internalPointer())
		return;

	if(historyCurrentIndex != historySize-1)
	{
		for(int i = historyCurrentIndex + 1; i < historySize; i++)
			history.removeAt(i);

		historySize = history.size();

		ui->treeForwardButton->setEnabled(false);
	}

	history << index;
	historyCurrentIndex++;
	historySize++;

	if(historyCurrentIndex > 0)
		ui->treeBackButton->setEnabled(true);
}

void MainWindow::historyBack()
{
	const QModelIndex index = history[--historyCurrentIndex];
	ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());
	Item *item = static_cast<Item*>(index.internalPointer());

	ui->treeLeft->setCurrentIndex(index);
	sm->requestTechSpecs(item);
	fm->setRootIndex(index);

	ui->treeBackButton->setEnabled( !(historyCurrentIndex == 0) );
	ui->treeForwardButton->setEnabled(true);

	ui->dirTreePathLineEdit->setText(item->server->name() + item->pathRelativeToDataSource());
}

void MainWindow::historyForward()
{
	const QModelIndex index = history[++historyCurrentIndex];
	ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());
	Item *item = static_cast<Item*>(index.internalPointer());

	ui->treeLeft->setCurrentIndex(index);
	sm->requestTechSpecs(item);
	fm->setRootIndex(index);

	ui->treeForwardButton->setEnabled( !(historyCurrentIndex == historySize-1) );
	ui->treeBackButton->setEnabled(true);

	ui->dirTreePathLineEdit->setText(item->server->name() + item->pathRelativeToDataSource());
}

QString MainWindow::getCurrentLanguageCode()
{
	QString lang = settings->value("Language").toString();
	return (lang.isEmpty() || lang == "detect") ? QLocale::system().name() : lang;
}

QString MainWindow::getCurrentMetadataLanguageCode()
{
	if(currentMetadataLang.isEmpty())
		return getCurrentLanguageCode();
	return currentMetadataLang;
}

void MainWindow::showOrHideProductView()
{
	if(settings->value("Extensions/ProductView/Enabled", false).toBool())
	{
		if(!productView)
		{
			productView = new ProductView(ui->tabWidget);
			//ui->tabWidget->addTab(productView, tr("ProductView"));

			connect(ui->tree, SIGNAL(activated(QModelIndex)),
			        this, SLOT(previewInProductView(QModelIndex)));
			connect(ui->tree, SIGNAL(clicked(QModelIndex)),
			        this, SLOT(previewInProductView(QModelIndex)));
			connect(ui->tree, SIGNAL(doubleClicked(QModelIndex)),
			        this, SLOT(tree_doubleClicked(QModelIndex)));
			connect(static_cast<ServersModel*>(ui->treeLeft->model()), SIGNAL(fileDownloaded(File*)),
			        productView, SLOT(fileDownloaded(File*)));
		}
	} else if (productView)
	{
		productView->deleteLater();
		productView = 0;
	}
}

void MainWindow::previewInProductView(const QModelIndex &index)
{
	if(!productView)
		return;

	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->tree->model())->mapToSource(index);

	File *f = fm->getRootItem()->files.at(srcIndex.row());

	if (!productView->expectFile(f))
		return;

	//ui->tabWidget->setCurrentIndex(PRODUCT_VIEW);

	// local files are not copied - just open the original
	// remote datasources are cached in ~/.cache/... or something similar
	if (f->parentItem->server->dataSource == LOCAL)
	{
		f->cachePath = f->path;
		productView->fileDownloaded(f);
	}
	else if (f->parentItem->server->dataSource == FTP)
	{
		QString pathForItem(f->parentItem->server->getPathForItem(f->parentItem));
		QDir d;
		d.mkpath(pathForItem);

		f->cachePath = pathForItem + "/" + f->name;
		// simulate "real cache" hit. Use local copy of the file (if it exists)
		if (QFile::exists(f->cachePath))
		{
			productView->fileDownloaded(f);
		}
		else
		{
			f->transferHandler = DownloadModel::ServersModel;
			downloading = true;
			f->parentItem->server->downloadFiles(QList<File*>() << f, pathForItem);
		}
	}
	else
	{
		Q_ASSERT_X(0, "business logic error", "Unhandled dataSource type");
	}

	if (productView->canHandle())
	{
		productView->show();
		// keep focus on the main window - keyboard handling
		activateWindow();
	}
	else
		productView->hide();
}

void MainWindow::tree_doubleClicked(const QModelIndex &index)
{
	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->tree->model())->mapToSource(index);

	File *f = fm->getRootItem()->files.at(srcIndex.row());

	QDesktopServices::openUrl(QUrl::fromLocalFile(f->path));
}
