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
#include <QWebSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversmodel.h"
#include "downloadmodel.h"
#include "filtersdialog.h"
#include "item.h"
#include "zimautils.h"
#include "errordialog.h"
#include "settings.h"


QString MainWindow::currentMetadataLang;

MainWindow::MainWindow(QTranslator *translator, QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindowClass),
	  translator(translator),
	  historyCurrentIndex(-1),
      historySize(0)
{
	qApp->setWindowIcon(QIcon(":/gfx/icon.png"));

    bool useSplash = Settings::get()->GUISplashEnabled;
    QSplashScreen *splash = 0;

	if( useSplash )
	{
		QPixmap pixmap(":/gfx/splash.png");

		splash = new QSplashScreen(pixmap);
		splash->setMask(pixmap.mask());
		splash->show();
	}

	ui->setupUi(this);

	ui->actionHistoryBack->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	ui->actionHistoryForward->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));

	connect(ui->serversWidget, SIGNAL(showSettings(SettingsDialog::Section)),
	        this, SLOT(showSettings(SettingsDialog::Section)));
	connect(ui->serversWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(trackHistory(const QModelIndex&)));
	connect(ui->serversWidget, SIGNAL(activated(const QModelIndex&)), this, SLOT(trackHistory(const QModelIndex&)));

	connect(ui->actionHistoryBack, SIGNAL(triggered()), this, SLOT(historyBack()));
	connect(ui->actionHistoryForward, SIGNAL(triggered()), this, SLOT(historyForward()));

	connect(ui->actionHome, SIGNAL(triggered()), this, SLOT(goToWorkingDirectory()));

    connect(ui->actionFilters, SIGNAL(triggered()),
            this, SLOT(setFiltersDialog()));

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

    restoreState(Settings::get()->MainWindowState);
    restoreGeometry(Settings::get()->MainWindowGeometry);

	connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
	connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(setWorkingDirectoryDialog()));
	connect(ui->openWorkDirButton, SIGNAL(clicked()), this, SLOT(openWorkingDirectory()));


	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);


	// status bar - use this one
	connect(ui->serversWidget, SIGNAL(statusUpdated(QString)),
	        this, SLOT(updateStatus(QString)));
	connect(ui->serversWidget, SIGNAL(autoDescentProgress(QModelIndex)),
            this, SLOT(autoDescentProgress(QModelIndex)));
	connect(ui->serversWidget, SIGNAL(autoDescentComplete(QModelIndex)),
            this, SLOT(autoDescendComplete(QModelIndex)));
	connect(ui->serversWidget, SIGNAL(autoDescentNotFound()),
            this, SLOT(autoDescentNotFound()));

	connect(ui->serversWidget, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));


#warning "TODO/FIXME: server model"
#if 0
	if (serversModel->loadQueue(settings) )
		ui->startStopDownloadBtn->setText(tr("Resume"));
#endif

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

		ui->toolBar->addWidget(flag);
	}

	if (cnt)
		ui->toolBar->addSeparator();

	connect(langButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeLanguage(int)));

	// History
	ui->actionHistoryBack->setEnabled(false);
	ui->actionHistoryForward->setEnabled(false);

#ifdef Q_OS_MAC
	QMenu *menu = new QMenu(this);
	QAction *settingsAction = new QAction(tr("Preferences"), this);
	settingsAction->setMenuRole(QAction::PreferencesRole);
	connect(settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));

	menu->addAction(settingsAction);
	menuBar()->addMenu(menu);
#endif

#warning TODO/FIXME: refactoring
	// Make shortcut Ctrl+C or Cmd+C available
//	QAction *copyAction = ui->techSpec->pageAction(QWebPage::Copy);
//	copyAction->setShortcut(QKeySequence::Copy);
//	ui->techSpec->addAction(copyAction);

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

	goToWorkingDirectory();

	QAction *act = new QAction(this);
	act->setShortcut(QKeySequence("Ctrl+Q"));

	connect(act, SIGNAL(triggered()), qApp, SLOT(quit()));

	addAction(act);

	if( useSplash )
	{
        SleeperThread::msleep(Settings::get()->GUISplashDuration);
		splash->finish(this);
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);
    }
    else
        QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Settings::get()->MainWindowState = saveState();
    Settings::get()->MainWindowGeometry = saveGeometry();
    Settings::get()->WorkingDir = ui->editDir->text();

#warning "TODO/FIXME: saveQueue"
    //serversModel->saveQueue(settings);

    Settings::get()->save();

	QMainWindow::closeEvent(e);
}

void MainWindow::setWorkingDirectoryDialog()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("ZIMA-CAD-Parts - set working directory"), ui->editDir->text());
	if (!str.isEmpty())
	{
        Settings::get()->HomeDir = "";
        Settings::get()->WorkingDir = str;
		ui->editDir->setText(str);

        settingsChanged();
	}
}

void MainWindow::showSettings(SettingsDialog::Section section)
{
    SettingsDialog sd(&translator, this);
    sd.setSection(section);

    if (sd.exec())
	{
        Settings::get()->WorkingDir = ui->editDir->text();
        settingsChanged();
    }
}

void MainWindow::settingsChanged()
{
    ui->editDir->setText(Settings::get()->WorkingDir);
    Settings::get()->recalculateFilters();

    const QMetaObject *mo;
    foreach (QWidget *w, findChildren<QWidget*>())
    {
        mo = w->metaObject();
        int settingsMethod = mo->indexOfMethod(QMetaObject::normalizedSignature("void settingsChanged()"));
        if (settingsMethod == -1)
        {
            qDebug() << "META> 'settingsChanged() method not found for" << w->objectName();
            continue;
        }
        mo->invokeMethod(w, "settingsChanged", Qt::DirectConnection);
    }


#warning TODO/FIXME: reload data sources
        //QList<BaseDataSource*> oldServers = servers;

//		servers = settingsDlg->getDatasources();
//		serversModel->setServerData(servers);

		int langIndex = SettingsDialog::langIndex(getCurrentLanguageCode()) - 1;

		langButtonGroup->button(langIndex)->setChecked(true);
		changeLanguage(langIndex);

		// Prune tree history
		history.clear();
		historyCurrentIndex = -1;
		historySize = 0;

		ui->actionHistoryBack->setEnabled(false);
		ui->actionHistoryForward->setEnabled(false);
}

void MainWindow::searchClicked()
{
	QMessageBox::information(this, "ZimaParts", "Not yet implemented");
}

void MainWindow::updateStatus(const QString &message)
{
	statusDir->setText(message);
}

void MainWindow::errorOccured(const QString &error)
{
	updateStatus(tr("Error: %1").arg(error));
	QMessageBox::warning(this, tr("FTP error"), error);
}

void MainWindow::filesDownloaded()
{
	updateStatus(tr("Parts downloaded."));
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

    ui->serversWidget->retranslateMetadata();
}

void MainWindow::autoDescentProgress(const QModelIndex &index)
{
	Item *item = static_cast<Item*>(index.internalPointer());

	qDebug() << "Progress expand" << index << item->name << item->parent->name;

	lastFoundIndex = index;
	ui->serversWidget->expand(index);
}

void MainWindow::autoDescendComplete(const QModelIndex &index)
{
	ui->serversWidget->expand(index);
    ui->serversWidget->setModelindex(index);
	trackHistory(index);
}

void MainWindow::autoDescentNotFound()
{
	if(lastFoundIndex.isValid())
	{
        ui->serversWidget->setModelindex(lastFoundIndex);
	}

	QMessageBox::warning(this, tr("Directory not found"), tr("Directory not found: %1").arg(autoDescentPath));
}

void MainWindow::goToWorkingDirectory()
{
    autoDescentPath = Settings::get()->HomeDir;

	if (autoDescentPath.isEmpty())
		return;
#warning "TODO/FIXME serversModel"
//	serversModel->descentTo(autoDescentPath);
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

		ui->actionHistoryForward->setEnabled(false);
	}

	history << index;
	historyCurrentIndex++;
	historySize++;

	if(historyCurrentIndex > 0)
		ui->actionHistoryBack->setEnabled(true);
}

void MainWindow::historyBack()
{
	const QModelIndex index = history[--historyCurrentIndex];
    ui->serversWidget->setModelindex(index);
	ui->actionHistoryBack->setEnabled( !(historyCurrentIndex == 0) );
	ui->actionHistoryForward->setEnabled(true);
}

void MainWindow::historyForward()
{
	const QModelIndex index = history[++historyCurrentIndex];
    ui->serversWidget->setModelindex(index);
	ui->actionHistoryForward->setEnabled( !(historyCurrentIndex == historySize-1) );
	ui->actionHistoryBack->setEnabled(true);
}

QString MainWindow::getCurrentLanguageCode()
{
    QString lang = Settings::get()->Language;
	return (lang.isEmpty() || lang == "detect") ? QLocale::system().name() : lang;
}

QString MainWindow::getCurrentMetadataLanguageCode()
{
	if(currentMetadataLang.isEmpty())
		return getCurrentLanguageCode();
	return currentMetadataLang;
}

void MainWindow::setFiltersDialog()
{
    FiltersDialog dlg;

    if (dlg.exec())
    {
        settingsChanged();
    }
}
