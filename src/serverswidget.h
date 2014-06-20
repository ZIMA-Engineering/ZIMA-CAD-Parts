#ifndef SERVERSWIDGET_H
#define SERVERSWIDGET_H

#include <QToolBox>
#include <QPushButton>
#include <QTreeView>

#include "ui_serverswidget.h"
#include "serversmodel.h"
#include "settingsdialog.h"
#include "servertabwidget.h"

class QSignalMapper;


class ServersWidgetMap
{
public:
	~ServersWidgetMap()
	{
		view->deleteLater();
		model->deleteLater();
		tab->deleteLater();
	}

	int index;
	ServersModel *model;
	QTreeView *view;
	ServerTabWidget *tab;
};


class ServersWidget : public QWidget, public Ui::ServersWidget
{
	Q_OBJECT
public:
	explicit ServersWidget(QWidget *parent = 0);
	//void setModel(ServersModel *model);

	void retranslateMetadata();

	void refresh(Item* item);
	void deleteFiles();
	void uncheckAll();

	QModelIndex currentIndex();
	void setModelindex(const QModelIndex &index);

signals:
	void statusUpdated(const QString &message);
	void showSettings(SettingsDialog::Section);

	void itemLoaded(const QModelIndex&);
	void clicked(const QModelIndex&);
	void activated(const QModelIndex&);
	// emit newly selected tree view root item to MainWindow to load toplevel files into part view
	//void groupChanged(const QModelIndex&);

	void errorOccured(const QString &error);
	void filesDownloaded();
	void fileDownloaded(File*);

	void techSpecAvailable(const QUrl&);
	void autoDescentProgress(const QModelIndex&);
	void autoDescentComplete(const QModelIndex&);
	void autoDescentNotFound();

	void workingDirChanged();

public slots:
	void expand(const QModelIndex & index);
	void settingsChanged();
	void goToWorkingDirectory();
	void retranslateMetadata(int langIndex);

private:
	QList<ServersWidgetMap*> m_map;

	QSignalMapper *m_signalMapper;
	QStringList m_zimaUtils;

private slots:
	void dirTreeContextMenu(QPoint point);
	void spawnZimaUtilityOnDir(int i);

	void loadingItem(Item *i);
	void allItemsLoaded();
	void setWorkingDirectory();
	void indexOpenPath();
	void splitterMoved(int, int);
};

#endif // SERVERSWIDGET_H
