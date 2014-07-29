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

#include "serversmodel.h"
#include <QDir>
#include <QDebug>
#include "basedatasource.h"
#include "baseremotedatasource.h"
#include "localdatasource.h"
#include "settings.h"
#include "downloadmodel.h"

ServersModel::ServersModel(BaseDataSource *ds, QObject *parent)
	: QAbstractItemModel(parent)
{
	m_lastTechSpecRequest = 0;
	m_datasource = ds;

	m_downloadModel = new DownloadModel(this, this);

	connect(this, SIGNAL(fileProgress(File*)), m_downloadModel, SLOT(fileChanged(File*)));
	connect(this, SIGNAL(fileDownloaded(File*)), m_downloadModel, SLOT(fileDownloaded(File*)));

	m_rootItem = new Item();
	m_rootItem->children.clear();
	m_rootItem->name = ds->label;
	m_rootItem->isServer = true;
	m_rootItem->server = ds;
	m_rootItem->server->rootItem = m_rootItem;
	connect(m_rootItem->server, SIGNAL(loadingItem(Item*)), this, SIGNAL(loadingItem(Item*)));
	connect(m_rootItem->server, SIGNAL(itemLoaded(Item*)), this, SLOT(allPartsDownloaded(Item*)));
	connect(m_rootItem->server, SIGNAL(allItemsLoaded()), this, SIGNAL(allItemsLoaded()));
	connect(m_rootItem->server, SIGNAL(techSpecAvailable(QUrl)), this, SIGNAL(techSpecAvailable(QUrl)));
	connect(m_rootItem->server, SIGNAL(statusUpdated(QString)), this, SIGNAL(statusUpdated(QString)));
	connect(m_rootItem->server, SIGNAL(fileProgress(File*)), this, SIGNAL(fileProgress(File*)));
	connect(m_rootItem->server, SIGNAL(fileDownloaded(File*)), this, SIGNAL(fileDownloaded(File*)));
	connect(m_rootItem->server, SIGNAL(filesDownloaded()), this, SLOT(dataSourceFinishedDownloading()));
	connect(m_rootItem->server, SIGNAL(metadataInclude(Item*,QString)), this, SLOT(metadataInclude(Item*,QString)));
	connect(m_rootItem->server, SIGNAL(metadataIncludeCancelled(Item*)), this, SLOT(metadataIncludeCancel(Item*)));
	connect(m_rootItem->server, SIGNAL(metadataReady(Item*)), this, SLOT(metadataReady(Item*)));
	connect(m_rootItem->server, SIGNAL(itemInserted(Item*)), this, SLOT(newItem(Item*)));
	connect(m_rootItem->server, SIGNAL(updateAvailable(Item*)), this, SLOT(itemUpdated(Item*)));
	connect(m_rootItem->server, SIGNAL(errorOccured(QString)), this, SIGNAL(errorOccured(QString)));
	connect(m_rootItem->server, SIGNAL(techSpecsIndexAlreadyExists(Item*)), this, SIGNAL(techSpecsIndexAlreadyExists(Item*)));
	connect(m_rootItem->server, SIGNAL(partsIndexAlreadyExists(Item*)), this, SIGNAL(partsIndexAlreadyExists(Item*)));
	connect(m_rootItem->server, SIGNAL(fileError(BaseDataSource::Operation,BaseDataSource::Error*)), this, SLOT(catchFileError(BaseDataSource::Operation,BaseDataSource::Error*)));
	connect(m_rootItem->server, SIGNAL(filesDeleted()), this, SLOT(dataSourceFinishedDeleting()));

	switch( m_rootItem->server->dataSource )
	{
	case LOCAL: {
		LocalDataSource *lds = static_cast<LocalDataSource*>(m_rootItem->server);
		m_rootItem->path = lds->localPath.endsWith('/') ? lds->localPath : lds->localPath + "/";
		break;
	}
	case UNDEFINED:
		// FIXME
		break;
	default:
		BaseRemoteDataSource *rds = static_cast<BaseRemoteDataSource*>(m_rootItem->server);
		m_rootItem->path = rds->remoteBaseDir.endsWith('/') ? rds->remoteBaseDir : rds->remoteBaseDir + "/";
	}

	ds->loadRootItem(m_rootItem);
	loadItem(m_rootItem);

	//reset();
	beginResetModel();
	endResetModel();
}

ServersModel::~ServersModel()
{
	delete m_rootItem;

	for(int i = 0; i < BaseDataSource::OperationCount; i++)
		qDeleteAll(m_fileErrors[i]);
}

bool ServersModel::canFetchMore(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return false;

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
		return m_rootItem->children.size();
	}

	Item* item = static_cast<Item*>(parent.internalPointer());

	//loadItem(item);

	//qDebug() << "asking about" << item->name << "returning " << (item->children.size() == 0 ? 1 : item->children.size());
	return item->children.size() == 0 ? 1 : item->children.size();
}

int ServersModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
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

	if (parentItem == m_rootItem || !parentItem)
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
		parentItem = m_rootItem;
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
		return tr("Data sources");

	return QVariant();
}

//nemenitelne
bool ServersModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(value);
	Q_UNUSED(role);
	return false;
}

QVariant ServersModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	Item* item = static_cast<Item*>(index.internalPointer());

	switch(role)
	{
	case Qt::DisplayRole:
		if(item->showText)
			return item->isServer ? item->name : item->getLabel();
		break;
	case Qt::DecorationRole:
		if(!item->logo.isNull())
			return item->logo;
		return item->server->itemIcon(item);
		break;
	case Qt::SizeHintRole:
		if(!item->logo.isNull())
			return QSize(item->logo.width(), item->logo.height());
		break;
	case Qt::ToolTipRole:
		return item->pathWithDataSource();
	default:
		;
	}

	return QVariant();
}

bool ServersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);
	return false;
}

Qt::ItemFlags ServersModel::flags(const QModelIndex &index) const
{
	return QAbstractItemModel::flags(index);
}

QString ServersModel::translateDataSourceNameToPath(QString name)
{
	if (m_datasource->name() == name)
		return m_datasource->pathToDataRoot();
	return QString();
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

	if(!autoDescents.isEmpty())
	{
		foreach(TreeAutoDescent *descent, autoDescents)
		{
			if(descent->waitsFor(item))
			{
				qDebug() << "Continue descent";
				descent->continueDescent(true);
				break;
			}
		}
	}
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

	requestTechSpecs(static_cast<Item*>(index.internalPointer()));
}

void ServersModel::requestTechSpecs(Item *item)
{
	item->server->sendTechSpecUrl(item);

	m_lastTechSpecRequest = item;
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

	Q_ASSERT(item);
	Q_ASSERT(item->server);
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

void ServersModel::deleteFiles()
{
	qDeleteAll(m_fileErrors[BaseDataSource::Delete]);
	m_fileErrors[BaseDataSource::Delete].clear();

	dsDeleted = 0;

	foreach(Item *i, m_rootItem->children)
	i->server->deleteFiles(getCheckedFiles(i));
}

void ServersModel::downloadFiles(QString dir)
{
	QDir d;
	d.mkpath(dir);

	foreach(Item *i, m_rootItem->children)
	{
		QList<File*> tmp = getCheckedFiles(i);

		foreach(File *f, tmp)
		f->transferHandler = DownloadModel::ServersModel;

		m_downloadModel->enqueue(tmp);

		if( tmp.count() > 0 )
			i->server->downloadFiles(tmp, dir);
	}
}

void ServersModel::resumeDownload()
{
	foreach(Item *i, m_rootItem->children)
	i->server->resumeDownload();
}

void ServersModel::uncheckAll(Item *item)
{
	if( item == 0 )
		item = m_rootItem;

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
	m_downloadModel->clear();

	foreach(Item *i, m_rootItem->children)
	i->server->deleteDownloadQueue();

	emit queueChanged();
}

#warning "TODO/FIXME: void ServersModel::saveQueue"
#if 0
void ServersModel::saveQueue(QSettings *settings)
{
	settings->remove("DownloadQueue");
	settings->beginGroup("DownloadQueue");

	QList<File*> files = downloadQueue->files();
	int i = 0;

	foreach(File *f, files)
	{
		if(!f->parentItem)
			continue;

		settings->beginGroup(QString::number(i));
		settings->setValue("DataSourceIndex", servers.indexOf(f->parentItem->server));
		settings->setValue("Name", f->name);
		settings->setValue("RemotePath", f->path);
		settings->setValue("TargetPath", f->targetPath);
		settings->setValue("Size", f->size);
		settings->endGroup();

		i++;
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
		f->transferHandler = DownloadModel::ServersModel;

		downloadQueue->enqueue(f);

		servers[dsIndex]->addFileToDownload(f);

		settings->endGroup();

		fileCnt++;
	}

	settings->endGroup();

	return fileCnt;
}
#endif
void ServersModel::retranslateMetadata(Item *item)
{
	QString lang = Settings::get()->getCurrentLanguageCode().left(2);
	if (!item)
	{
		item = m_rootItem;

		m_datasource->retranslate(lang);

		if(m_lastTechSpecRequest)
		{
			m_lastTechSpecRequest->server->sendTechSpecUrl(m_lastTechSpecRequest);
			m_lastTechSpecRequest->server->assignThumbnailsToFiles(m_lastTechSpecRequest);
		}
	}

	foreach(Item *i, item->children)
	{
		if(i->metadata)
		{
			i->metadata->retranslate(lang);
			itemUpdated(i); // Maybe we should send only one signal for all items
		}

		retranslateMetadata(i);
	}
}

void ServersModel::dataSourceFinishedDownloading()
{
	if( m_downloadModel->isEmpty() )
		emit filesDownloaded();
}

void ServersModel::dataSourceFinishedDeleting()
{
	if(++dsDeleted == m_rootItem->children.size())
		emit filesDeleted();
}

void ServersModel::metadataReady(Item *item)
{
	QModelIndex index = createIndex(item->row(), 0, item);

	emit dataChanged(index, index);
}

void ServersModel::newItem(Item *item)
{
	QModelIndex index = createIndex(item->parent->row(), 0, item);

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

void ServersModel::forwardAutoDescentProgress(TreeAutoDescent *descent, Item *item)
{
	if(!metadataIncludeHash.contains(descent))
		emit autoDescentProgress(createIndex(item->row(), 0, item));
}

void ServersModel::forwardAutoDescentCompleted(TreeAutoDescent *descent, Item *item)
{
	autoDescents.removeOne(descent);

	if(metadataIncludeHash.contains(descent))
	{
		if(metadataIncludeHash[descent])
			metadataIncludeHash[descent]->metadata->provideInclude(item->metadata);

		metadataIncludeHash.remove(descent);

	} else
		emit autoDescentCompleted(createIndex(item->row(), 0, item));

	descent->deleteLater();
}

void ServersModel::forwardAutoDescentNotFound(TreeAutoDescent *descent)
{
	autoDescents.removeOne(descent);

	if(metadataIncludeHash.contains(descent))
	{
		if(metadataIncludeHash[descent])
			metadataIncludeHash[descent]->metadata->provideInclude(0, descent->path());

		metadataIncludeHash.remove(descent);

	} else
		emit autoDescentNotFound();

	descent->deleteLater();
}

void ServersModel::metadataInclude(Item *item, QString path)
{
	descentTo(path, item);
}

void ServersModel::metadataIncludeCancel(Item *item)
{
	QHashIterator<TreeAutoDescent*, Item*> i(metadataIncludeHash);

	while(i.hasNext())
	{
		i.next();

		if(i.value() == item)
			metadataIncludeHash[i.key()] = 0;
	}
}

void ServersModel::abort()
{
	m_datasource->abort();
}

void ServersModel::descentTo(QString path, Item *item)
{
	if(path.isEmpty())
		return;

	TreeAutoDescent *descent = new TreeAutoDescent(this, m_rootItem, path, this);

	connect(descent, SIGNAL(progress(TreeAutoDescent*,Item*)), this, SLOT(forwardAutoDescentProgress(TreeAutoDescent*,Item*)));
	connect(descent, SIGNAL(completed(TreeAutoDescent*,Item*)), this, SLOT(forwardAutoDescentCompleted(TreeAutoDescent*,Item*)));
	connect(descent, SIGNAL(notFound(TreeAutoDescent*)), this, SLOT(forwardAutoDescentNotFound(TreeAutoDescent*)));

	autoDescents << descent;

	if(item)
		metadataIncludeHash[descent] = item;

	descent->descend();
}

QList<BaseDataSource::Error*> ServersModel::fileErrors(BaseDataSource::Operation op)
{
	return m_fileErrors[op];
}

bool ServersModel::hasErrors(BaseDataSource::Operation op)
{
	return !m_fileErrors[op].isEmpty();
}

Item* ServersModel::lastTechSpecRequest()
{
	return m_lastTechSpecRequest;
}

void ServersModel::stopDownload()
{
	abort();
}

void ServersModel::clearQueue()
{
	foreach(Item *i, m_rootItem->children)
	i->server->deleteDownloadQueue();
}

void ServersModel::catchFileError(BaseDataSource::Operation op, BaseDataSource::Error *err)
{
	m_fileErrors[op] << err;
}
