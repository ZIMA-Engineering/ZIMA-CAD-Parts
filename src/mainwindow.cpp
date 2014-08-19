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
#include <QTextStream>
#include <QDebug>
#include <QWebHistory>
#include <QWebSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversmodel.h"
#include "filtersdialog.h"
#include "zimautils.h"
#include "settings.h"
#include "languageflagswidget.h"
#include "workingdirwidget.h"


MainWindow::MainWindow(QTranslator *translator, QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindowClass),
	  translator(translator),
      m_historyCurrent(-1)
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

	connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    ui->actionRefresh->setShortcut(QKeySequence::Refresh);
    ui->actionRefresh->setShortcutContext(Qt::ApplicationShortcut);
    connect(ui->actionRefresh, SIGNAL(triggered()),
            MetadataCache::get(), SLOT(clear()));

#ifdef Q_WS_WIN
    // do not display menu bar for now on Windows. It contains only one "File" item now.
	// On th eother side it's mandatory to allow user to quit the app in some
	// X11 window manager (and mac)
	ui->menuBar->hide();
#endif

	ui->actionHistoryBack->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	ui->actionHistoryForward->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));

	connect(ui->serversWidget, SIGNAL(showSettings(SettingsDialog::Section)),
	        this, SLOT(showSettings(SettingsDialog::Section)));

    connect(ui->serversWidget, SIGNAL(directorySelected(QString)),
            this, SLOT(trackHistory(QString)));

	connect(ui->actionHistoryBack, SIGNAL(triggered()), this, SLOT(historyBack()));
	connect(ui->actionHistoryForward, SIGNAL(triggered()), this, SLOT(historyForward()));

	connect(ui->actionHome, SIGNAL(triggered()),
	        ui->serversWidget, SLOT(goToWorkingDirectory()));

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

	restoreState(Settings::get()->MainWindowState);
	restoreGeometry(Settings::get()->MainWindowGeometry);

	QList<int> list;
	list << (int)(width()*0.25) << (int)(width()*0.75);
	ui->splitter->setSizes(list);

	// status bar - use this one
	connect(ui->serversWidget, SIGNAL(statusUpdated(QString)),
	        this, SLOT(updateStatus(QString)));

	connect(ui->serversWidget, SIGNAL(workingDirChanged()), this, SLOT(settingsChanged()));

	m_wdirWidget = new WorkingDirWidget(this);
	ui->toolBar->addWidget(m_wdirWidget);

	ui->toolBar->addSeparator();

	LanguageFlagsWidget *flagsWidget = new LanguageFlagsWidget(this);
	ui->toolBar->addWidget(flagsWidget);

	ui->toolBar->addSeparator();
	ui->toolBar->addAction(ui->action_Preferences);
	connect(ui->action_Preferences, SIGNAL(triggered()), this, SLOT(showSettings()));

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

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

	QMainWindow::closeEvent(e);
}

void MainWindow::showSettings(SettingsDialog::Section section)
{
	SettingsDialog sd(&translator, this);
	sd.setSection(section);

	if (sd.exec())
		settingsChanged();
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
	ui->serversWidget->settingsChanged();
	m_wdirWidget->settingsChanged();

	// Prune tree history
    m_history.clear();
    m_historyCurrent = -1;

	ui->actionHistoryBack->setEnabled(false);
	ui->actionHistoryForward->setEnabled(false);
}

void MainWindow::updateStatus(const QString &message)
{
	statusDir->setText(message);
}

void MainWindow::trackHistory(const QString &path)
{
    if (m_history.count() && m_history.at(0) == path)
        return;
    else
    {
        m_history << path;
        m_historyCurrent++;
    }

    if (m_historyCurrent == 0)
		ui->actionHistoryForward->setEnabled(false);
    else if (m_historyCurrent > 0)
		ui->actionHistoryBack->setEnabled(true);
}

void MainWindow::historyBack()
{
    --m_historyCurrent;
    handleHistory();
}

void MainWindow::historyForward()
{
    ++m_historyCurrent;
    handleHistory();
}

void MainWindow::handleHistory()
{
    qDebug() << "handle history" << m_historyCurrent << m_history.at(m_historyCurrent);
    QString path = m_history.at(m_historyCurrent);
    ui->serversWidget->setDirectory(path);

    ui->actionHistoryBack->setEnabled(m_historyCurrent != 0);
    ui->actionHistoryForward->setEnabled(m_historyCurrent != m_history.count()-1);
}
