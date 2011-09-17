#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversmodel.h"
#include "filemodel.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindowClass)
{
	ui->setupUi(this);

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

	settingsDlg = new SettingsDialog(this);

	settings = new QSettings(QDir::homePath() + "/.zimaparts/settings.conf", QSettings::IniFormat, this);
	ui->editDir->setText(settings->value("workingDir", QDir::homePath()).toString() + "/.zimaparts");
	restoreState(settings->value("state", QByteArray()).toByteArray());
	restoreGeometry(settings->value("geometry", QByteArray()).toByteArray());

	connect(ui->btnDownload, SIGNAL(clicked()), this, SLOT(downloadButton()));
	connect(ui->btnSettings, SIGNAL(clicked()), this, SLOT(showSettings()));
	connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(setWorkingDirectory()));
	connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
	connect(ui->btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));

	//connect(ui->tree, SIGNAL(expanded(const QModelIndex&)), this, SLOT(treeExpandedOrCollaped(const QModelIndex&)));
	//connect(ui->tree, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(treeExpandedOrCollaped(const QModelIndex&)));
	//connect(ui->treeLeft, SIGNAL(clicked(QModelIndex)), this, SLOT(serverSelected(QModelIndex)));
	connect(ui->treeLeft, SIGNAL(clicked(const QModelIndex&)), this, SLOT(serverSelected(const QModelIndex&)));
	connect(ui->treeLeft, SIGNAL(expanded(const QModelIndex&)), this, SLOT(serverSelected(const QModelIndex&)));

	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);

	settingsDlg->loadSettings(settings);
	servers = settingsDlg->getData();

	//    PartsModel *pm = new PartsModel(this);
	//    connect(pm, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));
	//    connect(pm, SIGNAL(serverLoaded()), this, SLOT(serverLoaded()));
	//    pm->setDirIcon(style()->standardIcon(QStyle::SP_DirLinkIcon), style()->standardIcon(QStyle::SP_DirIcon));
	//    pm->setShowEmpty(settings->value("gui/showEmpty", true).toBool());
	//    pm->updateModel();
	//    ui->tree->setModel(pm);

	ServersModel *sm = new ServersModel(this);
	sm->setIcons(style()->standardIcon(QStyle::SP_DirIcon), style()->standardIcon(QStyle::SP_DriveNetIcon));
	sm->setServerData(*servers);
	ui->treeLeft->setModel(sm);

	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), this, SLOT(itemLoaded(const QModelIndex&)));
	connect(sm, SIGNAL(techSpecAvailable(QUrl)), this, SLOT(loadTechSpec(QUrl)));
	connect(sm, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));

	FileModel* fm = new FileModel(this);
	//fm->setSourceModel(sm);
	//fm->setDynamicSortFilter(true);
	fm->setThumbnailSize(settings->value("gui/thumbWidth").toInt());
	ui->tree->setModel(fm);

	connect(sm, SIGNAL(itemLoaded(const QModelIndex&)), fm, SLOT(setRootIndex(const QModelIndex&)));
	connect(sm, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
	connect(sm, SIGNAL(filesDownloaded()), this, SLOT(filesDownloaded()));

	currentServer = 0;

	ui->techSpec->load(QUrl("qrc:/zimaparts.html"));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	settings->setValue("state", saveState());
	settings->setValue("geometry", saveGeometry());
	QMainWindow::closeEvent(e);
}

void MainWindow::downloadButton()
{
	//    PartsModel *pm = static_cast<PartsModel*>(ui->tree->model());
	//    pm->downloadSelected(ui->editDir->text());

	FileModel *fm = static_cast<FileModel*>(ui->tree->model());
	fm->downloadFiles( fm->getRootItem(), ui->editDir->text() );
}

void MainWindow::setWorkingDirectory()
{
	QString str = QFileDialog::getExistingDirectory(this, ("Zima Parts - set working directory"), ui->editDir->text());
	if (!str.isEmpty())
	{
		settings->setValue("workingDir", str);
		ui->editDir->setText(str);
	}
}

void MainWindow::showSettings()
{
	//settingsDlg->loadSettings(settings);
	int result = settingsDlg->exec();
	if (result == QDialog::Accepted)
	{
		settingsDlg->saveSettings(settings);
		//((PartsModel*)ui->tree->model())->setShowEmpty(settings->value("gui/showEmpty", true).toBool());
		ServersModel* sm = static_cast<ServersModel*>(ui->treeLeft->model());
		FileModel* fm = static_cast<FileModel*>(ui->tree->model());

		sm->setServerData(*servers);
		fm->setThumbnailSize(settings->value("gui/thumbWidth").toInt());
	}
}

void MainWindow::updateClicked()
{
	Item* i = static_cast<FileModel*>(ui->tree->model())->getRootItem();

	if( i == 0)
		return;

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
		static_cast<ServersModel*>( ui->treeLeft->model() )->abort();
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

	static_cast<FileModel*>( ui->tree->model() )->uncheckAll();
}
