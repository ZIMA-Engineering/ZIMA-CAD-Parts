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
#include <QSortFilterProxyModel>
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

#ifdef INCLUDE_PRODUCT_VIEW
#include "extensions/productview/productview.h"
#endif // INCLUDE_PRODUCT_VIEW

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
	static QString getCurrentLanguageCode();
	static QString getCurrentMetadataLanguageCode();

	struct Filter
	{
		Filter(File::FileTypes type) : type(type){}

		File::FileTypes type;
		bool enabled;
	};

	struct FilterGroup
	{
		FilterGroup(QString internalName, QString label) : internalName(internalName), label(label), enabled(0){}

		QString label;
		QString internalName;
		QList<Filter> filters;
		bool enabled;
	};

	static QList<FilterGroup> filterGroups;

private:
	enum Tabs {
		TECH_SPECS,
		PARTS,
		DOWNLOADS,
#ifdef INCLUDE_PRODUCT_VIEW
		PRODUCT_VIEW,
#endif // INCLUDE_PRODUCT_VIEW
		TABS_COUNT
	};

	Ui::MainWindowClass *ui;
	static QSettings           *settings;
	QVector<BaseDataSource*> servers;
	QLabel              *statusState, *statusDir;
	FtpDataSource           *currentServer;
	FileModel *fm;
	QSortFilterProxyModel *proxy;
	QTranslator *translator;
	bool downloading;
	QButtonGroup *langButtonGroup;
	static QString currentMetadataLang;
	QStringList langs;
	QList<QPushButton*> langFlags;
	QSignalMapper *dirTreeSignalMapper;

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

	QStringList zimaUtils;

#ifdef INCLUDE_PRODUCT_VIEW
	ProductView *productView;

	void showOrHideProductView();
#endif // INCLUDE_PRODUCT_VIEW

	bool s_developerMode;

	void loadExtensions();
	void setupDeveloperMode();
	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent*);

public slots:
	void downloadButton();
	void setWorkingDirectoryDialog();
	void showSettings(SettingsDialog::Section section = SettingsDialog::General);
	void updateClicked();
	void deleteSelectedParts();
	void searchClicked();
	void serverSelected(const QModelIndex&);
	void updateStatus(QString);
	void treeExpandedOrCollaped();
	void loadingItem(Item *item);
	void itemLoaded(const QModelIndex&);
	void allItemsLoaded();
	void loadTechSpec(QUrl url);
	void errorOccured(QString error);
	void filesDownloaded();
	void setFiltersDialog();
	void rebuildFilters();
	void toggleDownload();
	void resumeDownload();
	void stopDownload();
	void loadAboutPage();
	void filesDeleted();

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
	void descentTo();
	void autoDescentProgress(const QModelIndex &index);
	void autoDescendComplete(const QModelIndex &index);
	void autoDescentNotFound();
	void adjustThumbColumnWidth(int width);
	void assignUrlToDirectory(bool overwrite = false);
	void techSpecsIndexOverwrite(Item *item);
	void assignPartsIndexUrlToDirectory(bool overwrite = false);
	void partsIndexOverwrite(Item *item);
	void openDirTreePath();
	void loadSettings();
	void loadZimaUtils();
	QVector<BaseDataSource*> loadDataSources();
	void saveSettings();
	void loadFilters();
	void saveFilters();
	void dirTreeContextMenu(QPoint point);
	void spawnZimaUtilityOnDir(int i);
	void setWorkingDirectory();
	void goToWorkingDirectory();
	void trackHistory(const QModelIndex &index);
	void historyBack();
	void historyForward();

#ifdef INCLUDE_PRODUCT_VIEW
	void previewInProductView(const QModelIndex &index);
#endif // INCLUDE_PRODUCT_VIEW
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
