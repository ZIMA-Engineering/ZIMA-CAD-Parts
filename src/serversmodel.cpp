/*
  ZIMA-Parts
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

#include "serversmodel.h"
#include <QDir>
#include <QDebug>
#include "baseremotedatasource.h"
#include "localdatasource.h"

ServersModel::ServersModel(QObject *parent) : QAbstractItemModel(parent)
{
	rootItem = new Item();

//	ftpData = new FtpData(this);

//	ftpData->changeSettings("localhost", 21, true, "", "", "/");
//	connect(ftpData, SIGNAL(allPartsDownloaded(Item*)), this, SLOT(allPartsDownloaded(Item*)));
	//	connect(ftpData, SIGNAL(statusUpdated(QString)), this, SLOT(updateStatus(QString)));
	//	connect(ftpData, SIGNAL(updateAvailable()), this, SIGNAL(layoutChanged()));
}

ServersModel::~ServersModel()
{
	delete rootItem;
}

bool ServersModel::canFetchMore(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return false;

	Item* item = static_cast<Item*>(parent.internalPointer());

//	if(!item->children.isEmpty() || !item->files.isEmpty())
//		return false;

//	if(item->hasLoadedChildren)
//		return false;

	return true;
}

void ServersModel::fetchMore(const QModelIndex &parent)
{
	if(!parent.isValid())
		return;

	Item* item = static_cast<Item*>(parent.internalPointer());

	loadItem(item);
}

//bool ServersModel::hasChildren(const QModelIndex &parent) const
//{
//	qDebug() << "Do I have children? :)";



//	return true;
//}

//pocet riadkov spadajucich pod parent
int ServersModel::rowCount(const QModelIndex &parent) const
{
	//qDebug() << "rowCount";

	if (!parent.isValid())
	{
		//qDebug() << "Not valid, returning" << rootItem->children.size();
		return rootItem->children.size();
	}

	Item* item = static_cast<Item*>(parent.internalPointer());

	//loadItem(item);

	//qDebug() << "asking about" << item->name << "returning " << (item->children.size() == 0 ? 1 : item->children.size());
	return item->children.size() == 0 ? 1 : item->children.size();
}

int ServersModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}

QModelIndex ServersModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	Item *i = static_cast<Item*>(index.internalPointer());
	Item *parentItem = i->parent;

	//qDebug() << "parent";
	//qDebug() << i->name;

	if (parentItem == rootItem || !parentItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

QModelIndex ServersModel::index(int row, int column, const QModelIndex &parent) const
{
	//qDebug() << "index";

	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	Item *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<Item*>(parent.internalPointer());

	Item *i = parentItem->child(row);
	//qDebug() << "returning index to" << (i?i->name:"parent");
	if (i)
		return createIndex(row, column, i);
	else
		return QModelIndex();
}

//data v hlavicke, v podstate budu zavisiet od nastaveni stlpcov
QVariant ServersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	if (section == 0)
		return tr("Server");

	return QVariant();
}

//nemenitelne
bool ServersModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	return false;
}

QVariant ServersModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	//	if (role == Qt::DisplayRole)
	//	{
	//		if (index.column() == 0)
	//			return servers->value(index.row())->address;
	//	}else if (role == Qt::DecorationRole)
	//	{
	//		return serverIcon;
	//		/*if (index.column() == 1)
	//            return part->pixmap;*/
	//	}

	Item* item = static_cast<Item*>(index.internalPointer());

	switch(role)
	{
	case Qt::DisplayRole:
		if(item->showText)
			return item->isServer ? item->name : item->getLabel();
		break;
	case Qt::DecorationRole:
//		if( item->isServer )
//			return serverIcon;
//		else if( item->isDir )
//			return dirIcon;
		if(!item->logo.isNull())
			return item->logo;
		return item->server->itemIcon(item);
		break;
	case Qt::SizeHintRole:
		if(!item->logo.isNull())
			return QSize(item->logo.width(), item->logo.height());
		break;
	default:;
	}

	return QVariant();
}

bool ServersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	return false;
}

Qt::ItemFlags ServersModel::flags(const QModelIndex &index) const
{
	return QAbstractItemModel::flags(index);
}

void ServersModel::setServerData(QVector<BaseDataSource*> srv)
{
	//qDeleteAll(rootItem->children);
	rootItem->children.clear();
	servers = srv;

	foreach(BaseDataSource *s, servers)
	{
		Item *i = new Item();
		i->name = s->label;
		i->isServer = true;
		i->server = s;
		i->server->rootItem = i;
//		i->server->changeSettings(i->server->remoteHost,
//					i->server->remotePort,
//					i->server->passiveMode,
//					i->server->login,
//					i->server->password,
//					i->server->baseDir);
		connect(i->server, SIGNAL(loadingItem(Item*)), this, SIGNAL(loadingItem(Item*)));
		connect(i->server, SIGNAL(itemLoaded(Item*)), this, SLOT(allPartsDownloaded(Item*)));
		connect(i->server, SIGNAL(allItemsLoaded()), this, SIGNAL(allItemsLoaded()));
		connect(i->server, SIGNAL(techSpecAvailable(QUrl)), this, SIGNAL(techSpecAvailable(QUrl)));
		connect(i->server, SIGNAL(statusUpdated(QString)), this, SIGNAL(statusUpdated(QString)));
		connect(i->server, SIGNAL(fileProgress(File*)), this, SIGNAL(fileProgress(File*)));
		connect(i->server, SIGNAL(fileDownloaded(File*)), this, SIGNAL(fileDownloaded(File*)));
		connect(i->server, SIGNAL(filesDownloaded()), this, SLOT(dataSourceFinishedDownloading()));
		connect(i->server, SIGNAL(metadataReady(Item*)), this, SLOT(metadataReady(Item*)));
		connect(i->server, SIGNAL(itemInserted(Item*)), this, SLOT(newItem(Item*)));
		connect(i->server, SIGNAL(updateAvailable(Item*)), this, SLOT(itemUpdated(Item*)));
		connect(i->server, SIGNAL(errorOccured(QString)), this, SIGNAL(errorOccured(QString)));

		i->parent = rootItem;

		switch( i->server->dataSource )
		{
		case LOCAL: {
			LocalDataSource *lds = static_cast<LocalDataSource*>(i->server);
			i->path = lds->localPath.endsWith('/') ? lds->localPath : lds->localPath + "/";
			break;
		}
		case UNDEFINED:
			// FIXME
			break;
		default:
			BaseRemoteDataSource *rds = static_cast<BaseRemoteDataSource*>(i->server);
			i->path = rds->remoteBaseDir.endsWith('/') ? rds->remoteBaseDir : rds->remoteBaseDir + "/";
		}

		s->loadRootItem(i);

		rootItem->children.append(i);
	}
	reset();
}

void ServersModel::allPartsDownloaded(Item* item)
{
	//emit statusUpdated(tr("All done."));
	//reset(); // ???
	//emit serverLoaded();
	//emit layoutChanged();
	if(item->children.count())
		item->isEmpty = false;
	emit layoutChanged();
	emit itemLoaded( createIndex(item->row(), 0, item) );
}

void ServersModel::refresh(Item* item)
{
	//qDebug() << "refresh";

	QModelIndex index = createIndex(item->row(), 0, item);
	beginRemoveRows(index, 0, item->children.count());

	qDeleteAll(item->children);
	item->children.clear();
	qDeleteAll(item->files);
	item->files.clear();

	//qDebug() << "resetting" << item->name;

	//qDebug() << "layoutChanged";

//	emit dataChanged(index, index);

	//emit rowsRemoved(index, 0, cnt);
	endRemoveRows();

	//reset();

	item->hasLoadedChildren = false;

	loadItem(item);
}

void ServersModel::requestTechSpecs(const QModelIndex &index)
{
	if(!index.isValid())
		return;
	Item *i = static_cast<Item*>(index.internalPointer());

	i->server->sendTechSpecUrl(i);
}

void ServersModel::loadItem(Item* item)
{
	//clear();

	if(item->hasLoadedChildren)
	{
		//qDebug() << "Item" << item->name << "has already loaded children - stop";

		//item->server->sendTechSpecUrl(item);
		allPartsDownloaded(item);
		return;
	} else {
		//qDebug() << "Item" << item->name << "has not yet loaded children - go on";
	}

//	if( item->children.size() > 0 || item->files.size() > 0 )
//	{
//		item->server->sendTechSpecUrl(item);
//		allPartsDownloaded(item);
//		return;
//	}

	// FIXME: may be missing when commented... we'll see
//	item->server->ftpData->changeSettings(item->server->address,
//				item->server->port,
//				item->server->passiveMode,
//				item->server->login,
//				item->server->password,
//				item->server->baseDir);
	item->server->loadDirectory(item);
}

QList<File*> ServersModel::getCheckedFiles(Item *item)
{
	QList<File*> ret;

	foreach(File *f, item->files)
		if( f->isChecked )
			ret << f;

	foreach(Item *i, item->children)
		ret << getCheckedFiles(i);

	return ret;
}

void ServersModel::downloadFiles(QString dir)
{
	QDir d;
	d.mkpath(dir);

	foreach(Item *i, rootItem->children)
	{
		QList<File*> tmp = getCheckedFiles(i);
		downloadQueue << tmp;

		if( tmp.count() > 0 )
			i->server->downloadFiles(tmp, dir);
	}

	emit newDownloadQueue(&downloadQueue);
}

void ServersModel::downloadSpecificFile(QString dir, File *f)
{
	QDir d;
	d.mkpath(dir);

	QList<File*> tmp;
	tmp << f;
	f->parentItem->server->downloadFiles(tmp, dir);

	downloadQueue << tmp;
	emit newDownloadQueue(&downloadQueue);
}

void ServersModel::resumeDownload()
{
	foreach(Item *i, rootItem->children)
		i->server->resumeDownload();
}

void ServersModel::uncheckAll(Item *item)
{
	if( item == 0 )
		item = rootItem;

	foreach(File* f, item->files)
		f->isChecked = false;

	foreach(Item *i, item->children)
		uncheckAll(i);

	//emit dataChanged( index(0, 0), index(rootItem->files.size(), 0) );
}

void ServersModel::clear()
{
	// FIXME
//	foreach(BaseDataSource *s, servers)
//	{
//		s->reset();
//	}
	//reset(); // ???
}

void ServersModel::deleteDownloadQueue()
{
	downloadQueue.clear();

	foreach(Item *i, rootItem->children)
		i->server->deleteDownloadQueue();

	emit queueChanged();
}

void ServersModel::saveQueue(QSettings *settings)
{
	settings->remove("DownloadQueue");
	settings->beginGroup("DownloadQueue");

	int cnt = downloadQueue.count();

	for(int i = 0; i < cnt; i++)
	{
		settings->beginGroup(QString::number(i));
		settings->setValue("DataSourceIndex", servers.indexOf(downloadQueue.at(i)->parentItem->server));
		settings->setValue("Name", downloadQueue.at(i)->name);
		settings->setValue("RemotePath", downloadQueue.at(i)->path);
		settings->setValue("TargetPath", downloadQueue.at(i)->targetPath);
		settings->setValue("Size", downloadQueue.at(i)->size);
		settings->endGroup();
	}

	settings->endGroup();
}

int ServersModel::loadQueue(QSettings *settings)
{
	int fileCnt = 0;

	settings->beginGroup("DownloadQueue");

	QStringList groups = settings->childGroups();
	int cnt = groups.count();
	int dsCnt = servers.count();

	for(int i = 0; i < cnt; i++)
	{
		settings->beginGroup(groups[i]);

		int dsIndex = settings->value("DataSourceIndex").toInt();

		if( dsIndex >= dsCnt )
			continue;

		File *f = new File;
		f->parentItem = servers[dsIndex]->getRootItem();
		f->name = settings->value("Name").toString();
		f->path = settings->value("RemotePath").toString();
		f->targetPath = settings->value("TargetPath").toString();
		f->size = settings->value("Size").toULongLong();
		f->bytesDone = 0;

		downloadQueue.append(f);

		servers[dsIndex]->addFileToDownload(f);

		settings->endGroup();

		fileCnt++;
	}

	settings->endGroup();

	emit newDownloadQueue(&downloadQueue);

	return fileCnt;
}

void ServersModel::dataSourceFinishedDownloading()
{
	if( downloadQueue.isEmpty() )
		emit filesDownloaded();
}

void ServersModel::metadataReady(Item *item)
{
	QModelIndex index = createIndex(item->row(), 0, item);

	emit dataChanged(index, index);
}

void ServersModel::newItem(Item *item)
{
	QModelIndex index = createIndex(item->parent->row(), 0, item->parent);

	beginInsertRows(index, item->children.count(), item->children.count()+1);
	//qDebug() << "insert" << item->name;
	endInsertRows();
	//emit dataChanged(index, index);
}

void ServersModel::itemUpdated(Item *item)
{
	QModelIndex index = createIndex(item->row(), 0, item);

	emit dataChanged(index, index);
}

void ServersModel::abort()
{
	foreach(BaseDataSource *s, servers)
	{
		s->abort();
	}
}
