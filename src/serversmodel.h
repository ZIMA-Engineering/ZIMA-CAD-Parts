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

#ifndef SERVERSMODEL_H
#define SERVERSMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QKeyEvent>

#include "item.h"
#include "basedatasource.h"
#include "treeautodescent.h"


#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QFileIconProvider>

class ServersModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit ServersModel(QObject *parent = 0);

    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

signals:

public slots:

};

class ServersIconProvider : public QFileIconProvider
{
public:
    ServersIconProvider();

    virtual QIcon   icon ( IconType type ) const;
    virtual QIcon   icon ( const QFileInfo & info ) const;
    virtual QString type ( const QFileInfo & info ) const;
};


class ServersProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ServersProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const;
};

#if 0
class DownloadModel;

class ServersModel : public QAbstractItemModel
{
	Q_OBJECT

public:
    ServersModel(LocalDataSource *ds, QObject *parent = 0);
	~ServersModel();

	bool canFetchMore(const QModelIndex &parent) const;
	void fetchMore(const QModelIndex &parent);
//	bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QModelIndex parent(const QModelIndex &index) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

    LocalDataSource* dataSource() {
		return m_datasource;
	}

	QString translateDataSourceNameToPath(QString name);
	QList<BaseDataSource::Error*> fileErrors(BaseDataSource::Operation op);
	bool hasErrors(BaseDataSource::Operation op);
	Item *lastTechSpecRequest();

	//! @returns datasource top level item
	Item *rootItem() {
		return m_rootItem;
	}

public slots:
	void clear();
	void loadItem(Item *item);
	void requestTechSpecs(const QModelIndex &index);
	void requestTechSpecs(Item *item);
	void deleteFiles();
    void copyToWorkingDir();
	void uncheckAll(Item *item = 0);
	void retranslateMetadata(Item *item = 0);
	void descentTo(QString path, Item *item = 0);
	void catchFileError(BaseDataSource::Operation op, BaseDataSource::Error *err);

protected:
	QList<File*> getCheckedFiles(Item *item);

protected slots:
	void allPartsDownloaded(Item* item);

private:
	QIcon dirIcon, serverIcon;
    LocalDataSource* m_datasource;
	Item *m_rootItem;
	Item *m_lastTechSpecRequest;
	QList<BaseDataSource::Error*> m_fileErrors[BaseDataSource::OperationCount];
	int dsDeleted;
	QList<TreeAutoDescent*> autoDescents;
	QHash<TreeAutoDescent*, Item*> metadataIncludeHash;

private slots:
	void dataSourceFinishedDeleting();
	void metadataReady(Item *item);
	void newItem(Item *item);
	void itemUpdated(Item *item);
	void forwardAutoDescentProgress(TreeAutoDescent *descent, Item *item);
	void forwardAutoDescentCompleted(TreeAutoDescent *descent, Item *item);
	void forwardAutoDescentNotFound(TreeAutoDescent *descent);
	void metadataInclude(Item *item, QString path);
	void metadataIncludeCancel(Item *item);

signals:
	void loadingItem(Item*);
	void itemLoaded(const QModelIndex&);
	void techSpecAvailable(QUrl);
	void statusUpdated(QString);
	void fileProgress(File*);
	void queueChanged();
	void partsIndexAlreadyExists(Item*);
	void filesDeleted();
};
#endif
#endif // SERVERSMODEL_H
