/*
  ZIMA-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011 Jakub Skokan <aither@havefun.cz>

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModelIndex>
#include <QLabel>
#include <QUrl>
#include <QSortFilterProxyModel>
#include <QThread>
#include <QTranslator>
#include "settingsdialog.h"
#include "filemodel.h"

namespace Ui
{
class MainWindowClass;
}

class QFtp;
class FtpDataSource;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QTranslator *translator, QWidget *parent = 0);
	~MainWindow();
	void keyPressEvent(QKeyEvent *event);

private:
	Ui::MainWindowClass *ui;
	QSettings           *settings;
	QVector<BaseDataSource*> servers;
	QLabel              *statusState, *statusDir;
	FtpDataSource           *currentServer;
	FileModel *fm;
	QSortFilterProxyModel *proxy;
	QTranslator *translator;
	bool downloading;

	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent*);

public slots:
	void downloadButton();
	void setWorkingDirectory();
	void showSettings();
	void updateClicked();
	void searchClicked();
	void serverSelected(const QModelIndex&);
	void updateStatus(QString);
	void treeExpandedOrCollaped(const QModelIndex&);
	void serverLoaded();
	void itemLoaded(const QModelIndex&);
	void loadTechSpec(QUrl url);
	void errorOccured(QString error);
	void filesDownloaded();
	void rebuildFilters();
	void toggleDownload();
	void resumeDownload();
	void stopDownload();
	void loadAboutPage();

private slots:
	void loadSettings();
	QVector<BaseDataSource*> loadDataSources();
	void saveSettings();
};

class SleeperThread : public QThread
{
public:
	static void msleep(unsigned long msecs)
	{
		QThread::msleep(msecs);
	}
};

#endif // MAINWINDOW_H
