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

#ifdef Q_WS_WIN
    // do not display menu bar for now on WIndows. It contains only one "File" item now.
    // On th eother side it's mandatory to allow user to quit the app in some
    // X11 window manager (and mac)
    ui->menuBar->hide();
#endif

	ui->actionHistoryBack->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	ui->actionHistoryForward->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));

	connect(ui->serversWidget, SIGNAL(showSettings(SettingsDialog::Section)),
	        this, SLOT(showSettings(SettingsDialog::Section)));
	connect(ui->serversWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(trackHistory(const QModelIndex&)));
	connect(ui->serversWidget, SIGNAL(activated(const QModelIndex&)), this, SLOT(trackHistory(const QModelIndex&)));

	connect(ui->actionHistoryBack, SIGNAL(triggered()), this, SLOT(historyBack()));
	connect(ui->actionHistoryForward, SIGNAL(triggered()), this, SLOT(historyForward()));

    connect(ui->actionHome, SIGNAL(triggered()),
            ui->serversWidget, SLOT(goToWorkingDirectory()));

	statusDir = new QLabel(tr("Ready"), this);
	statusDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	statusBar()->addWidget(statusDir, 100);

    restoreState(Settings::get()->MainWindowState);
    restoreGeometry(Settings::get()->MainWindowGeometry);

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
    connect(ui->serversWidget, SIGNAL(workingDirChanged()), this, SLOT(settingsChanged()));


#warning "TODO/FIXME: server model"
#if 0
	if (serversModel->loadQueue(settings) )
		ui->startStopDownloadBtn->setText(tr("Resume"));
#endif

    ui->toolBar->addSeparator();

    QString currentLang = Settings::get()->getCurrentLanguageCode().left(2);

	langButtonGroup = new QButtonGroup(this);
	langButtonGroup->setExclusive(true);

    foreach (QString lang, Settings::get()->Languages)
	{
        QString langCode = lang.left(2);

        QPushButton *flag = new QPushButton(QIcon(QString(":/gfx/flags/%1.png").arg(langCode)), "", this);
		flag->setFlat(true);
		flag->setCheckable(true);
		flag->setStyleSheet("width: 16px; height: 16px; margin: 0; padding: 1px;");

		if(currentLang == lang)
			flag->setChecked(true);

        langButtonGroup->addButton(flag, Settings::get()->Languages.indexOf(lang));

		ui->toolBar->addWidget(flag);
	}

    ui->toolBar->addSeparator();

	connect(langButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeLanguage(int)));

    // HACK: move settings actions to the right of the window
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->toolBar->addWidget(empty);

    ui->toolBar->addAction(ui->action_Preferences);
    connect(ui->action_Preferences, SIGNAL(triggered()), this, SLOT(showSettings()));

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

    settingsChanged();
    ui->serversWidget->goToWorkingDirectory();

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

#warning "TODO/FIXME: saveQueue"
    //serversModel->saveQueue(settings);

    Settings::get()->save();

	QMainWindow::closeEvent(e);
}

void MainWindow::setWorkingDirectoryDialog()
{
    QString str = QFileDialog::getExistingDirectory(this,
                                                    tr("ZIMA-CAD-Parts - set working directory"),
                                                    Settings::get()->WorkingDir);
	if (!str.isEmpty())
	{
        Settings::get()->WorkingDir = str;
        settingsChanged();
	}
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
    ui->workingDirectoryEdit->setText(Settings::get()->WorkingDir);
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

    int langIndex = SettingsDialog::langIndex(Settings::get()->getCurrentLanguageCode()) - 1;

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
    if(!QFile::exists(Settings::get()->WorkingDir))
	{
		QDir dir;

        if(!dir.mkpath(Settings::get()->WorkingDir))
		{
            QMessageBox::warning(this, tr("Unable to create working directory"),
                                 tr("Unable to create working directory: %1").arg(Settings::get()->WorkingDir));
			return;
		}
	}

    QDesktopServices::openUrl(QUrl::fromLocalFile(Settings::get()->WorkingDir));
}

void MainWindow::changeLanguage(int lang)
{
    if (Settings::get()->getCurrentLanguageCode() == Settings::get()->Languages[lang])
		return;

    ui->serversWidget->retranslateMetadata();
}

void MainWindow::autoDescentProgress(const QModelIndex &index)
{
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

    QMessageBox::warning(this, tr("Directory not found"), tr("Directory not found: %1").arg(Settings::get()->WorkingDir));
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
