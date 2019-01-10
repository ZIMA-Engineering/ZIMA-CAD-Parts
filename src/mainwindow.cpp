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
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include <QRegExp>
#include <QFile>
#include <QDebug>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QShortcut>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datasourcemodel.h"
#include "filtersdialog.h"
#include "zimautils.h"
#include "settings.h"
#include "datasourcewidget.h"


MainWindow::MainWindow(QTranslator *translator, QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindowClass),
	  translator(translator),
	  m_downloader(0)
{
	qApp->setWindowIcon(QIcon(":/gfx/icon.png"));

	QSplashScreen *splash = 0;

	if (Settings::get()->GUISplashEnabled)
	{
		QPixmap pixmap(":/gfx/splash.png");

		splash = new QSplashScreen(pixmap);
		splash->setMask(pixmap.mask());
		splash->show();
	}

	ui->setupUi(this);

	connect(ui->action_Preferences, SIGNAL(triggered()),
			this, SLOT(showSettings()));
	connect(ui->actionAbout_Qt, SIGNAL(triggered()),
			qApp, SLOT(aboutQt()));

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
	// do not display menu bar on Windows and Linux. It contains only one "File"
	// item now. On th eother side it's mandatory to allow user to quit the app
	// in some X11 window manager (and mac).
	ui->menuBar->hide();
#endif

	connect(ui->toolBar, SIGNAL(settingsRequested()),
			this, SLOT(showSettings()));

	connect(ui->tabWidget, SIGNAL(showSettings(SettingsDialog::Section)),
			this, SLOT(showSettings(SettingsDialog::Section)));

	connect(ui->tabWidget, SIGNAL(newHistory(DataSourceHistory*)),
			ui->toolBar, SLOT(setupHistory(DataSourceHistory*)));

	ui->toolBar->setupHistory(ui->tabWidget->currentDataSource()->history());

	restoreState(Settings::get()->MainWindowState);
	restoreGeometry(Settings::get()->MainWindowGeometry);

	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);

	connect(ui->tabWidget, SIGNAL(workingDirChanged()), this, SLOT(settingsChanged()));

	QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
	connect(QWebEngineProfile::defaultProfile(), SIGNAL(downloadRequested(QWebEngineDownloadItem*)),
			this, SLOT(downloadFile(QWebEngineDownloadItem*)));

	settingsChanged();

	if (Settings::get()->GUISplashEnabled)
	{
		SleeperThread::msleep(Settings::get()->GUISplashDuration);
		splash->finish(this);
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::showSettings(SettingsDialog::Section section)
{
	SettingsDialog sd(&translator, this);
	sd.setSection(section);

	if (sd.exec())
		settingsChanged();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Alt)
	{
		ui->menuBar->setVisible(!ui->menuBar->isVisible());
		return;
	}

	QMainWindow::keyPressEvent(event);
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
	if (!e->spontaneous())
	{
		Settings::get()->MainWindowState = saveState();
		Settings::get()->MainWindowGeometry = saveGeometry();
	}

	QMainWindow::closeEvent(e);
}

void MainWindow::settingsChanged()
{
	Settings::get()->recalculateFilters();

#if 0
	// it crashes sometimes...
	const QMetaObject *mo;
	foreach (QWidget *w, findChildren<QWidget*>())
	{
		mo = w->metaObject();
		int settingsMethod = mo->indexOfMethod(QMetaObject::normalizedSignature("settingsChanged()"));
		if (settingsMethod == -1)
		{
			qDebug() << "META> 'settingsChanged() method not found for" << w->objectName();
			continue;
		}
		else
		{
			qDebug() << "META> 'settingsChanged() method YES found for" << w->objectName();
			mo->invokeMethod(w, "settingsChanged", Qt::DirectConnection);
		}
	}
#endif

	ui->toolBar->settingsChanged();
	ui->tabWidget->settingsChanged();
}


void MainWindow::downloadFile(QWebEngineDownloadItem *download)
{
	QFileInfo fi(download->path());
	QString filePath = Settings::get()->getWorkingDir() + "/" + fi.fileName();

	qDebug() << "Downloading into" << filePath;

	if (QDir().exists(filePath))
	{
		if (QMessageBox::question(this, tr("File Exists"),
								  tr("File %1 already exists. Overwrite?").arg(filePath),
								  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		{
			download->cancel();
			return;
		}
	}

	download->setPath(filePath);
	download->accept();

	if (!m_downloader)
		m_downloader = new WebDownloaderDialog(this);

	m_downloader->enqueue(download);
	m_downloader->show();
}
