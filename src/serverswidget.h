#ifndef SERVERSWIDGET_H
#define SERVERSWIDGET_H

#include <QToolBox>
#include <QPushButton>

#include "serversmodel.h"
#include "settingsdialog.h"

class QSignalMapper;
class QTreeView;


class ServersWidget : public QToolBox
{
	Q_OBJECT
public:
	explicit ServersWidget(QWidget *parent = 0);
	//void setModel(ServersModel *model);
    void setDataSources(QList<BaseDataSource*> datasources);

	QModelIndex currentIndex();

signals:
	void showSettings(SettingsDialog::Section);
	void clicked(const QModelIndex&);
	void activated(const QModelIndex&);
	// emit newly selected tree view root item to MainWindow to load toplevel files into part view
	void groupChanged(const QModelIndex&);

public slots:
	void expand(const QModelIndex & index);
	void setCurrentIndex(const QModelIndex & index);

private:
	//ServersModel *m_model;
    QList<ServersModel*> m_models;
	QList<QTreeView*> m_views;

	QSignalMapper *m_signalMapper;
	QStringList m_zimaUtils;

private slots:
	void dirTreeContextMenu(QPoint point);
	void spawnZimaUtilityOnDir(int i);
	// handle current widget change
	void this_currentChanged(int i);
};

#endif // SERVERSWIDGET_H
