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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModelIndex>
#include <QLabel>
#include <QUrl>
#include <QThread>
#include <QTranslator>
#include <QButtonGroup>
#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QSignalMapper>

#include "settingsdialog.h"
#include "filemodel.h"
#include "zima-cad-parts.h"
#include "filefiltermodel.h"
#include "filefilters/filtergroup.h"


namespace Ui
{
class MainWindowClass;
}

class QFtp;
class FtpDataSource;
class DownloadModel;
class ProductView;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QTranslator *translator, QWidget *parent = 0);
	~MainWindow();
	void keyPressEvent(QKeyEvent *event);
	static QString getCurrentLanguageCode();
	static QString getCurrentMetadataLanguageCode();

private:
	enum Tabs {
	    TECH_SPECS,
	    PARTS,
	    DOWNLOADS,
//		PRODUCT_VIEW,
	    TABS_COUNT
	};

	Ui::MainWindowClass *ui;

    QLabel              *statusDir; // status bar
    FileModel *fm; // TODO/FIXME: to refactoring
    FileFilterModel *proxy; // TODO/FIXME: to refactoring
    QTranslator *translator;// app ui

    QButtonGroup *langButtonGroup; // toolbar
    static QString currentMetadataLang; // lang handling
    QStringList langs; // lang handling

	// History
	QList<QModelIndex> history;
	int historyCurrentIndex;
	int historySize;

	// Tech spec toolbar
	QToolBar *techSpecToolBar;
	QAction *techSpecBackAction;
	QAction *techSpecForwardAction;
	QLineEdit *urlBar;

	// Parts index toolbar
	QToolBar *partsIndexToolBar;
	QAction *partsIndexBackAction;
	QAction *partsIndexForwardAction;
	QLineEdit *partsIndexUrlBar;

	QString autoDescentPath;
	Item *lastPartsIndexItem;
	QUrl lastPartsIndex;
	QDateTime lastPartsIndexModTime;
	QModelIndex lastFoundIndex;

    ProductView *m_productView;

    void setupDeveloperMode(); // WTF?

	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent*);

public slots:
	void downloadButton();
	void setWorkingDirectoryDialog();
	void showSettings(SettingsDialog::Section section = SettingsDialog::General);
	void updateClicked();
	void deleteSelectedParts();
	void searchClicked();
	void updateStatus(const QString &message);
	void treeExpandedOrCollaped();
	void loadTechSpec(QUrl url);
	void errorOccured(const QString &error);
	void filesDownloaded();
	void setFiltersDialog();
	void rebuildFilters();
	void toggleDownload();
	void resumeDownload();
	void stopDownload();
	void filesDeleted(ServersModel*);

private slots:
	void openWorkingDirectory();
	void changeLanguage(int lang);
	void goToUrl();
	void updateUrlBar(QUrl url);
	void setPartsIndex(const QModelIndex &index);
	void partsIndexLoaded(const QModelIndex &index);
	void viewHidePartsIndex(Item *item = 0);
	void goToPartsIndexUrl();
	void updatePartsUrlBar(QUrl url);
	void autoDescentProgress(const QModelIndex &index);
	void autoDescendComplete(const QModelIndex &index);
	void autoDescentNotFound();
	void adjustThumbColumnWidth(int width);
	void assignUrlToDirectory(bool overwrite = false);
	void techSpecsIndexOverwrite(Item *item);
	void assignPartsIndexUrlToDirectory(bool overwrite = false);
	void partsIndexOverwrite(Item *item);
	void loadSettings();
	QList<BaseDataSource*> loadDataSources();
	void saveSettings();
	void loadFilters();
	void saveFilters();
	void setWorkingDirectory();
	void goToWorkingDirectory();
	void trackHistory(const QModelIndex &index);
	void historyBack();
	void historyForward();

	void previewInProductView(const QModelIndex &index);
	void tree_doubleClicked(const QModelIndex &index);
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
