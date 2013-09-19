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
#include <QVector>
#include <QKeyEvent>

#include "item.h"
#include "basedatasource.h"
#include "treeautodescent.h"

class ServersModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	ServersModel(QObject *parent = 0);
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
	//---
	void setServerData(QVector<BaseDataSource*>);
	QString translateDataSourceNameToPath(QString name);
	QList<BaseDataSource::Error*> fileErrors(BaseDataSource::Operation op);
	bool hasErrors(BaseDataSource::Operation op);

public slots:
	void refresh(Item* item);
	void clear();
	void loadItem(Item *item);
	void requestTechSpecs(const QModelIndex &index);
	void requestTechSpecs(Item *item);
	void deleteFiles();
	void downloadFiles(QString dir);
	void downloadSpecificFile(QString dir, File *f);
	void resumeDownload();
	void uncheckAll(Item *item = 0);
	void deleteDownloadQueue();
	void saveQueue(QSettings *settings);
	int loadQueue(QSettings *settings);
	void retranslateMetadata(Item *item = 0);
	void abort();
	void descentTo(QString path, Item *item = 0);
	void assignTechSpecUrlToItem(QString url, Item *item, bool overwrite = false);
	void assignPartsIndexUrlToItem(QString url, Item *item, bool overwrite = false);
	void catchFileError(BaseDataSource::Operation op, BaseDataSource::Error *err);

protected:
	QList<File*> getCheckedFiles(Item *item);

protected slots:
	void allPartsDownloaded(Item* item);

private:
	QIcon dirIcon, serverIcon;
	QVector<BaseDataSource*> servers;
	Item *rootItem;
	Item *lastTechSpecRequest;
	QList<File*> downloadQueue;
	QList<BaseDataSource::Error*> m_fileErrors[BaseDataSource::OperationCount];
	int dsDeleted;
	QList<TreeAutoDescent*> autoDescents;
	QHash<TreeAutoDescent*, Item*> metadataIncludeHash;

private slots:
	void dataSourceFinishedDownloading();
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
	void allItemsLoaded();
	void techSpecAvailable(QUrl);
	void statusUpdated(QString);
	void errorOccured(QString);
	void fileProgress(File*);
	void fileDownloaded(File*);
	void filesDownloaded();
	void newDownloadQueue(QList<File*>*);
	void queueChanged();
	void autoDescentProgress(const QModelIndex&);
	void autoDescentCompleted(const QModelIndex&);
	void autoDescentNotFound();
	void techSpecsIndexAlreadyExists(Item*);
	void partsIndexAlreadyExists(Item*);
	void filesDeleted();
};

#endif // SERVERSMODEL_H
