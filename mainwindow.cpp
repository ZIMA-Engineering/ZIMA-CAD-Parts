#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversmodel.h"
#include "downloadmodel.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include <QRegExp>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindowClass)
{
	downloading = false;

	settings = new QSettings(this);
	bool useSplash = settings->value("GUI/Splash/Enabled", true).toBool();
	QSplashScreen *splash;

	if( useSplash )
	{
		QPixmap pixmap(":/data/splash.png");
		splash = new QSplashScreen(pixmap);
		splash->setMask(pixmap.mask());
		splash->show();
	}

	ui->setupUi(this);

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

	//settingsDlg = new SettingsDialog(this);

	servers = loadDataSources();
	loadSettings();

	restoreState(settings->value("state", QByteArray()).toByteArray());
	restoreGeometry(settings->value("geometry", QByteArray()).toByteArray());

	connect(ui->btnDownload, SIGNAL(clicked()), this, SLOT(downloadButton()));
	connect(ui->btnSettings, SIGNAL(clicked()), this, SLOT(showSettings()));
	connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(setWorkingDirectory()));
	connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
	connect(ui->startStopDownloadBtn, SIGNAL(clicked()), this, SLOT(toggleDownload()));
	//connect(ui->btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));

	//connect(ui->tree, SIGNAL(expanded(const QModelIndex&)), this, SLOT(treeExpandedOrCollaped(const QModelIndex&)));
	//connect(ui->tree, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(treeExpandedOrCollaped(const QModelIndex&)));
	//connect(ui->treeLeft, SIGNAL(clicked(QModelIndex)), this, SLOT(serverSelected(QModelIndex)));
	connect(ui->treeLeft, SIGNAL(expanded(QModelIndex)), ui->treeLeft, SIGNAL(clicked(QModelIndex)));
	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), this, SLOT(serverSelected(const QModelIndex&)));
	//connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), this, SLOT(serverSelected(const QModelIndex&)));

	connect(ui->filtersListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(rebuildFilters()));

	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);

	//settingsDlg->loadSettings(settings);
	//servers = settingsDlg->getData();

	//    PartsModel *pm = new PartsModel(this);
	//    connect(pm, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));
	//    connect(pm, SIGNAL(serverLoaded()), this, SLOT(serverLoaded()));
	//    pm->setDirIcon(style()->standardIcon(QStyle::SP_DirLinkIcon), style()->standardIcon(QStyle::SP_DirIcon));
	//    pm->setShowEmpty(settings->value("gui/showEmpty", true).toBool());
	//    pm->updateModel();
	//    ui->tree->setModel(pm);

	ServersModel *sm = new ServersModel(this);
	sm->setIcons(style()->standardIcon(QStyle::SP_DirIcon), style()->standardIcon(QStyle::SP_DriveNetIcon));
	sm->setServerData(servers);
	ui->treeLeft->setModel(sm);

	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), this, SLOT(itemLoaded(const QModelIndex&)));
	connect(sm, SIGNAL(techSpecAvailable(QUrl)), this, SLOT(loadTechSpec(QUrl)));
	connect(sm, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));
	connect(ui->deleteQueueBtn, SIGNAL(clicked()), sm, SLOT(deleteDownloadQueue()));

	fm = new FileModel(this);
	//fm->setSourceModel(sm);
	//fm->setDynamicSortFilter(true);

	fm->setThumbWidth( settings->value("GUI/ThumbWidth", 32).toInt() );
	fm->setPreviewWidth( settings->value("GUI/PreviewWidth", 256).toInt() );

	proxy = new QSortFilterProxyModel(this);
	proxy->setSourceModel(fm);

	settings->beginGroup("PartFilters");
	int cnt = ui->filtersListWidget->count();
	for(int i = 0; i < cnt; i++)
		ui->filtersListWidget->item(i)->setCheckState( settings->value( QString::number(i), true ).toBool() ? Qt::Checked : Qt::Unchecked );
	settings->endGroup();

	rebuildFilters();

	ui->tree->setModel(proxy);

	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
	connect(sm, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
	connect(sm, SIGNAL(filesDownloaded()), this, SLOT(filesDownloaded()));

	DownloadModel *dm = new DownloadModel(this);
	ui->downloadTreeView->setModel(dm);
	ui->downloadTreeView->setItemDelegate(new DownloadDelegate(this));

	connect(sm, SIGNAL(newDownloadQueue(QList<File*>*)), dm, SLOT(setQueue(QList<File*>*)));
	connect(sm, SIGNAL(fileProgress(File*)), dm, SLOT(fileChanged(File*)));
	connect(sm, SIGNAL(fileDownloaded(File*)), dm, SLOT(fileDownloaded(File*)));
	connect(sm, SIGNAL(queueChanged()), dm, SLOT(queueChanged()));

	currentServer = 0;

	ui->techSpec->load(QUrl("qrc:/data/zimaparts.html"));

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

void MainWindow::closeEvent(QCloseEvent *e)
{
	settings->setValue("state", saveState());
	settings->setValue("geometry", saveGeometry());

	int cnt = ui->filtersListWidget->count();

	settings->beginGroup("PartFilters");
	for(int i = 0; i < cnt; i++)
		settings->setValue( QString::number(i), ui->filtersListWidget->item(i)->checkState() == Qt::Checked );
	settings->endGroup();

	QMainWindow::closeEvent(e);
}

void MainWindow::downloadButton()
{
	//    PartsModel *pm = static_cast<PartsModel*>(ui->tree->model());
	//    pm->downloadSelected(ui->editDir->text());

	ServersModel *sm = static_cast<ServersModel*>(ui->treeLeft->model());
	sm->downloadFiles( ui->editDir->text() );

	static_cast<ServersModel*>( ui->treeLeft->model() )->uncheckAll();
	ui->tabWidget->setCurrentIndex(2);

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
	SettingsDialog *settingsDlg = new SettingsDialog(settings, loadDataSources(), this);
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

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		ui->treeLeft->setEnabled(false);

		static_cast<ServersModel*>(ui->treeLeft->model())->loadItem(item);
	}
}

void MainWindow::updateStatus(QString str)
{
	statusDir->setText(str);
}

void MainWindow::itemLoaded(const QModelIndex &index)
{
	ui->treeLeft->expand(index);
	QApplication::restoreOverrideCursor();
	ui->treeLeft->setEnabled(true);
	ui->btnUpdate->setEnabled(true);
	treeExpandedOrCollaped(index);
}

void MainWindow::convertModelIndex(const QModelIndex &index)
{
//	if( !index.isValid() )
//		return;

//	FileModel* fm = static_cast<FileModel*>( ui->tree->model() );
//	QModelIndex new_index = fm->mapFromSource( index );

//	qDebug() << new_index << new_index.isValid();

//	Item* i = static_cast<Item*>(index.internalPointer());

//	ui->tree->setRootIndex(new_index);
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

		QApplication::restoreOverrideCursor();
		ui->treeLeft->setEnabled(true);
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

void MainWindow::rebuildFilters()
{
	QStringList expressions;

	for(int i = 0; i < File::TYPES_COUNT; i++)
	{
		/*
		  Realizováno by to bylo zaškrtávacími políčky v sekci parts.
Pro/Engineer(soubory *.prt a nebo *.prt(nejvyžší index)), CATIA(*.CATPart),
SolidWorks (*.sldprt), Blender (*.blend), *.iges(bude umět jak *.iges tak *.igs),
*.step(jak *.step tak *.stp), *.dwg, *.dxf.
		  */
		if( ui->filtersListWidget->item(i)->checkState() == Qt::Checked )
		{
			switch( i )
			{
			case File::PROE:
				expressions << ".+\\.prt\\.\\d+";
				break;
			case File::CATIA:
				expressions << ".+\\.CATPart";
				break;
			case File::IGES:
				expressions << ".+\\.iges|.+\\.igs";
				break;
			case File::STEP:
				expressions << ".+\\.step|.+\\.stp";
				break;
			case File::STL:
				expressions << ".+\\.stl";
				break;
			}
		}
	}

	QRegExp rx( "^" + expressions.join("|") + "$" );
	proxy->setFilterRegExp(rx);
}

void MainWindow::loadSettings()
{
//	settings->beginGroup("DataSources");
//	foreach(QString str, settings->childGroups())
//	{
//		settings->beginGroup(str);

//		QString dataSourceType = settings->value("dataSourceType", "ftp").toString();

//		if( dataSourceType == "ftp" )
//		{
//			FtpDataSource *s = new FtpDataSource();
//			s->loadSettings(*settings);
//			servers.append(s);
//		} else if ( dataSourceType == "local" ) {
//			LocalDataSource *s = new LocalDataSource();
//			s->loadSettings(*settings);
//			servers.append(s);
//		}

//		settings->endGroup();
//	}
//	settings->endGroup();

	ui->editDir->setText(settings->value("WorkingDir", QDir::homePath() + "/ZIMA-Parts").toString());
	//static_cast<FileModel*>( ui->tree->model() )->setThumbnailSize(settings->value("GUI/thumbWidth").toInt());
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

//			s->localPath = m_ui->pathLineEdit->text();
			s->saveSettings(*settings);
			break;
		}
		case FTP: {
			FtpDataSource *s = static_cast<FtpDataSource*>(bs);
//			s->remoteHost = m_ui->txtHost->text();
//			s->remotePort = m_ui->txtPort->text().toInt();
//			s->remoteBaseDir = m_ui->txtBaseDir->text();
//			s->remoteLogin = m_ui->txtLogin->text();
//			s->remotePassword = m_ui->txtPass->text();
//			s->ftpPassiveMode = m_ui->checkPassive->isChecked();

			s->saveSettings(*settings);
			break;
		}
		default:
			break;
		}

		settings->endGroup();
	}
	settings->endGroup();

	// FIXME
	//settings->setValue("GUI/thumbWidth", );
	settings->setValue("WorkingDir", ui->editDir->text());
	settings->sync();
}
