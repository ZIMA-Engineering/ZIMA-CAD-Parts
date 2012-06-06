/*
  ZIMA-Parts
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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversmodel.h"
#include "downloadmodel.h"
#include "filtersdialog.h"

QList<MainWindow::FilterGroup> MainWindow::filterGroups;
QSettings * MainWindow::settings;
QString MainWindow::currentMetadataLang;

MainWindow::MainWindow(QTranslator *translator, QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindowClass),
	  translator(translator)
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

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

	servers = loadDataSources();
	loadSettings();

	restoreState(settings->value("state", QByteArray()).toByteArray());
	restoreGeometry(settings->value("geometry", QByteArray()).toByteArray());

	connect(ui->btnDownload, SIGNAL(clicked()), this, SLOT(downloadButton()));
	connect(ui->btnSettings, SIGNAL(clicked()), this, SLOT(showSettings()));
	connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(setWorkingDirectory()));
	connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
	connect(ui->startStopDownloadBtn, SIGNAL(clicked()), this, SLOT(toggleDownload()));

	//connect(ui->treeLeft, SIGNAL(expanded(QModelIndex)), ui->treeLeft, SIGNAL(clicked(QModelIndex)));

	//connect(ui->filtersListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(rebuildFilters()));
	connect(ui->filtersButton, SIGNAL(clicked()), this, SLOT(setFiltersDialog()));

	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);


	ServersModel *sm = new ServersModel(this);
	sm->setServerData(servers);
	ui->treeLeft->setModel(sm);

	connect(sm, SIGNAL(loadingItem(Item*)), this, SLOT(loadingItem(Item*)));
	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), this, SLOT(itemLoaded(const QModelIndex&)));
	connect(sm, SIGNAL(allItemsLoaded()), this, SLOT(allItemsLoaded()));
	connect(sm, SIGNAL(techSpecAvailable(QUrl)), this, SLOT(loadTechSpec(QUrl)));
	connect(sm, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));
	connect(ui->deleteQueueBtn, SIGNAL(clicked()), sm, SLOT(deleteDownloadQueue()));

	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), sm, SLOT(requestTechSpecs(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(activated(const QModelIndex&)), sm, SLOT(requestTechSpecs(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), sm, SLOT(requestTechSpecs(const QModelIndex&)));

	fm = new FileModel(this);

	fm->setThumbWidth( settings->value("GUI/ThumbWidth", 32).toInt() );
	fm->setPreviewWidth( settings->value("GUI/PreviewWidth", 256).toInt() );

	proxy = new QSortFilterProxyModel(this);
	proxy->setSourceModel(fm);

	filterGroups << FilterGroup("ProE", "Pro/Engineer");
	filterGroups.last().filters
			<< Filter(File::PRT_PROE)
			<< Filter(File::ASM)
			<< Filter(File::DRW)
			<< Filter(File::FRM);

	filterGroups << FilterGroup("CATIA", "CATIA");
	filterGroups.last().filters
			<< Filter(File::CATPART)
			<< Filter(File::CATPRODUCT)
			<< Filter(File::CATDRAWING);

	filterGroups << FilterGroup("NX", "NX (UGS)");
	filterGroups.last().filters
			<< Filter(File::PRT_NX);

	filterGroups << FilterGroup("SolidWorks", "SolidWorks");
	filterGroups.last().filters
			<< Filter(File::SLDPRT)
			<< Filter(File::SLDPRT)
			<< Filter(File::SLDDRW);

	filterGroups << FilterGroup("SolidEdge", "Solid Edge");
	filterGroups.last().filters
			<< Filter(File::PAR)
			<< Filter(File::PSM)
			<< Filter(File::ASM)
			<< Filter(File::DFT);

	filterGroups << FilterGroup("Invertor", "INVERTOR");
	filterGroups.last().filters
			<< Filter(File::IPT)
			<< Filter(File::IAM)
			<< Filter(File::DWG);

	filterGroups << FilterGroup("CADNeutral", "CAD NEUTRAL");
	filterGroups.last().filters
			<< Filter(File::STEP)
			<< Filter(File::IGES)
			<< Filter(File::DWG)
			<< Filter(File::DXF);

	filterGroups << FilterGroup("NonCAD", "NonCAD");
	filterGroups.last().filters
			<< Filter(File::STL)
			<< Filter(File::BLEND);

	loadFilters();
	rebuildFilters();

	ui->tree->setModel(proxy);

	//connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
	connect(sm, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
	connect(sm, SIGNAL(filesDownloaded()), this, SLOT(filesDownloaded()));

	DownloadModel *dm = new DownloadModel(this);

	ui->downloadTreeView->setModel(dm);
	ui->downloadTreeView->setItemDelegate(new DownloadDelegate(this));

	connect(sm, SIGNAL(newDownloadQueue(QList<File*>*)), dm, SLOT(setQueue(QList<File*>*)));
	connect(sm, SIGNAL(fileProgress(File*)), dm, SLOT(fileChanged(File*)));
	connect(sm, SIGNAL(fileDownloaded(File*)), dm, SLOT(fileDownloaded(File*)));
	connect(sm, SIGNAL(queueChanged()), dm, SLOT(queueChanged()));

	if( sm->loadQueue(settings) )
		ui->startStopDownloadBtn->setText(tr("Resume"));

	currentServer = 0;

	langs << "en_US" << "cs_CZ";

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

	// Make shortcut Ctrl+C or Cmd+C available
	QAction *copyAction = ui->techSpec->pageAction(QWebPage::Copy);
	copyAction->setShortcut(QKeySequence::Copy);
	ui->techSpec->addAction(copyAction);

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

	loadAboutPage();

#ifdef INCLUDE_PRODUCT_VIEW
	productView = new ProductView(settings, this);

	showOrHideProductView();

	connect(ui->tree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(previewInProductView(QModelIndex)));
	connect(sm, SIGNAL(fileDownloaded(File*)), productView, SLOT(fileDownloaded(File*)));
#endif // INCLUDE_PRODUCT_VIEW

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

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);

		if( ui->techSpec->url().path().startsWith("/data/zima-parts") )
			loadAboutPage();
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

	QMainWindow::closeEvent(e);
}

void MainWindow::downloadButton()
{
	ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());
	sm->downloadFiles( ui->editDir->text() );

	static_cast<ServersModel*>( ui->treeLeft->model() )->uncheckAll();
	ui->tabWidget->setCurrentIndex(MainWindow::DOWNLOADS);

	int columnCnt = ui->downloadTreeView->model()->columnCount(QModelIndex());

	ui->downloadTreeView->resizeColumnToContents(0);
	ui->downloadTreeView->resizeColumnToContents(2);

	downloading = true;
}

void MainWindow::setWorkingDirectory()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("ZIMA-Parts - set working directory"), ui->editDir->text());
	if (!str.isEmpty())
	{
		settings->setValue("WorkingDir", str);
		ui->editDir->setText(str);
	}
}

void MainWindow::showSettings()
{
	SettingsDialog *settingsDlg = new SettingsDialog(settings, loadDataSources(), &translator, this);
	//settingsDlg->loadSettings(settings);
	int result = settingsDlg->exec();

	if (result == QDialog::Accepted)
	{
		settingsDlg->saveSettings();
		//((PartsModel*)ui->tree->model())->setShowEmpty(settings->value("gui/showEmpty", true).toBool());
		ServersModel* sm = static_cast<ServersModel*>(ui->treeLeft->model());
		fm->setRootIndex(QModelIndex());

		QVector<BaseDataSource*> oldServers = servers;

		servers = settingsDlg->getData();
		sm->setServerData(servers);

		qDeleteAll(oldServers);

		saveSettings();

		fm->setThumbWidth( settings->value("GUI/ThumbWidth", 32).toInt() );
		fm->setPreviewWidth( settings->value("GUI/PreviewWidth", 256).toInt() );

		loadAboutPage();

#ifdef INCLUDE_PRODUCT_VIEW
		showOrHideProductView();
#endif // INCLUDE_PRODUCT_VIEW
	}

	delete settingsDlg;
}

void MainWindow::updateClicked()
{
	Item* i = fm->getRootItem();

	if( i == 0)
		return;

	fm->setRootIndex(QModelIndex());

	static_cast<ServersModel*>( ui->treeLeft->model() )->refresh(i);
	ui->btnUpdate->setEnabled(false);
}

void MainWindow::searchClicked()
{
	QMessageBox::information(this, "ZimaParts", "Not yet implemented");
}

void MainWindow::treeExpandedOrCollaped(const QModelIndex &i)
{
	int columnCnt = ui->tree->model()->columnCount(QModelIndex());
	for (int i = 0; i < columnCnt; i++)
		ui->tree->resizeColumnToContents(i);
}

void MainWindow::serverLoaded()
{
//	for (int i=0; i<ui->tree->model()->columnCount(QModelIndex()); i++)
//		ui->tree->resizeColumnToContents(i);
}

void MainWindow::serverSelected(const QModelIndex &i)
{
	if (i.internalPointer())
	{
		Item* item = static_cast<Item*>(i.internalPointer());

//		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
//		ui->treeLeft->setEnabled(false);

		static_cast<ServersModel*>(ui->treeLeft->model())->loadItem(item);
	}
}

void MainWindow::updateStatus(QString str)
{
	statusDir->setText(str);
}

void MainWindow::loadingItem(Item *item)
{
	//QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	setCursor(QCursor(Qt::WaitCursor));

	statusDir->setText(tr("Loading %1...").arg(item->getLabel()));
}

void MainWindow::itemLoaded(const QModelIndex &index)
{
	//ui->treeLeft->expand(index);
//	QApplication::restoreOverrideCursor();
//	ui->treeLeft->setEnabled(true);
	ui->btnUpdate->setEnabled(true);
	treeExpandedOrCollaped(index);

	Item *i = static_cast<Item*>(index.internalPointer());

	if(i == fm->getRootItem())
		fm->setRootIndex(index);

	//qDebug() << "Loaded" << i->name;
}

void MainWindow::allItemsLoaded()
{
	//QApplication::restoreOverrideCursor();
	setCursor(QCursor(Qt::ArrowCursor));

	statusDir->setText(tr("All items loaded."));
}

void MainWindow::loadTechSpec(QUrl url)
{
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

		setCursor(QCursor(Qt::ArrowCursor));
		ui->btnUpdate->setEnabled(true);
		break;
	}
}

void MainWindow::errorOccured(QString error)
{
	QApplication::restoreOverrideCursor();
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
	static_cast<ServersModel*>( ui->treeLeft->model() )->resumeDownload();
	ui->startStopDownloadBtn->setText(tr("Stop"));
	downloading = true;
}

void MainWindow::stopDownload()
{
	static_cast<ServersModel*>( ui->treeLeft->model() )->abort();
	ui->startStopDownloadBtn->setText(tr("Resume"));
	downloading = false;
}

void MainWindow::toggleDownload()
{
	if( downloading )
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
			if(filterGroups[i].filters[j].enabled)
				expressions << File::getRxForFileType(filterGroups[i].filters[j].type);
		}
	}

	expressions.removeDuplicates();

	QRegExp rx( "^" + expressions.join("|") + "$" );
	proxy->setFilterRegExp(rx);
}

void MainWindow::loadAboutPage()
{
	QString url = ":/data/zima-parts%1.html";
	QString locale = settings->value("Language").toString();
	QString localized = url.arg("_" + (locale == "detect" ? QLocale::system().name() : locale));
	QString filename = (QFile::exists(localized) ? localized : url.arg("") );

	QFile f(filename);
	f.open(QIODevice::ReadOnly);
	QTextStream stream(&f);

	ui->techSpec->setHtml( stream.readAll().replace("%VERSION%", VERSION) );
}

void MainWindow::changeLanguage(int lang)
{
	if(getCurrentMetadataLanguageCode() == langs[lang])
		return;

	currentMetadataLang = langs[lang];

	static_cast<ServersModel*>(ui->treeLeft->model())->retranslateMetadata();
}

void MainWindow::loadSettings()
{
	ui->editDir->setText(settings->value("WorkingDir", QDir::homePath() + "/ZIMA-Parts").toString());
}

QVector<BaseDataSource*> MainWindow::loadDataSources()
{
	QVector<BaseDataSource*> servers;

	settings->beginGroup("DataSources");
	foreach(QString str, settings->childGroups())
	{
		settings->beginGroup(str);

		QString dataSourceType = settings->value("dataSourceType", "ftp").toString();

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
		{
			filterGroups[i].filters[j].enabled = settings->value(File::getInternalNameForFileType(filterGroups[i].filters[j].type), true).toBool();
		}

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
		{
			settings->setValue(File::getInternalNameForFileType(filterGroups[i].filters[j].type), filterGroups[i].filters[j].enabled);
		}

		settings->endGroup();
	}

	settings->endGroup();
}

QString MainWindow::getCurrentLanguageCode()
{
	QString lang = settings->value("Language").toString();
	return lang == "detect" ? QLocale::system().name() : lang;
}

QString MainWindow::getCurrentMetadataLanguageCode()
{
	if(currentMetadataLang.isEmpty())
		return getCurrentLanguageCode();
	return currentMetadataLang;
}

#ifdef INCLUDE_PRODUCT_VIEW
void MainWindow::showOrHideProductView()
{
	if(productView->isExtensionEnabled())
	{
		if(ui->tabWidget->indexOf(productView) == -1)
			ui->tabWidget->addTab(productView, tr("ProductView"));

		if(productView->isHidden())
			productView->show();
	} else {
		int index;
		if((index = ui->tabWidget->indexOf(productView)) != -1)
			ui->tabWidget->removeTab(index);

		productView->hide();
	}
}

void MainWindow::previewInProductView(const QModelIndex &index)
{
	if(!productView->isExtensionEnabled())
		return;

	QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(ui->tree->model())->mapToSource(index);

	File *f = fm->getRootItem()->files.at(srcIndex.row());

	if(f->type == File::PRT_PROE || f->type == File::PRT_NX)
	{
		productView->expectFile(f);

		static_cast<ServersModel*>(ui->treeLeft->model())->downloadSpecificFile(ui->editDir->text(), f);
		downloading = true;

		ui->tabWidget->setCurrentIndex(PRODUCT_VIEW);
	}
}

#endif // INCLUDE_PRODUCT_VIEW
