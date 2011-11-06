#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModelIndex>
#include <QLabel>
#include <QUrl>
#include <QSortFilterProxyModel>
#include <QThread>
#include "settingsdialog.h"
#include "settings.h"
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
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	void keyPressEvent(QKeyEvent *event);

private:
	Ui::MainWindowClass *ui;
	Settings      *baseSettings;
	QSettings           *settings;
	QVector<BaseDataSource*> servers;
	QLabel              *statusState, *statusDir;
	FtpDataSource           *currentServer;
	FileModel *fm;
	QSortFilterProxyModel *proxy;
	bool downloading;

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
	void convertModelIndex(const QModelIndex&);
	void loadTechSpec(QUrl url);
	void errorOccured(QString error);
	void filesDownloaded();
	void rebuildFilters();
	void toggleDownload();
	void resumeDownload();
	void stopDownload();

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
