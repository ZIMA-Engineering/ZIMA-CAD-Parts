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

//pocet riadkov spadajucich pod parent
int ServersModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
		return rootItem->children.size();

	Item* item = static_cast<Item*>(parent.internalPointer());

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

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

QModelIndex ServersModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	Item *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<Item*>(parent.internalPointer());

	Item *i;
	i = parentItem->child(row);

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
		return item->name;
		break;
	case Qt::DecorationRole:
//		if( item->isServer )
//			return serverIcon;
//		else if( item->isDir )
//			return dirIcon;
		return item->server->itemIcon(item);
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

void ServersModel::setIcons(QIcon dir, QIcon server)
{
	dirIcon = dir;
	serverIcon = server;
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
		connect(i->server, SIGNAL(allPartsDownloaded(Item*)), this, SLOT(allPartsDownloaded(Item*)));
		connect(i->server, SIGNAL(techSpecAvailable(QUrl)), this, SIGNAL(techSpecAvailable(QUrl)));
		connect(i->server, SIGNAL(statusUpdated(QString)), this, SIGNAL(statusUpdated(QString)));
		connect(i->server, SIGNAL(fileProgress(File*)), this, SIGNAL(fileProgress(File*)));
		connect(i->server, SIGNAL(fileDownloaded(File*)), this, SIGNAL(fileDownloaded(File*)));
		connect(i->server, SIGNAL(filesDownloaded()), this, SLOT(dataSourceFinishedDownloading()));
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

		rootItem->children.append(i);
	}
	reset();
}

void ServersModel::allPartsDownloaded(Item* item)
{
	emit statusUpdated(tr("All done."));
	//reset(); // ???
	//emit serverLoaded();
	//emit layoutChanged();
	emit layoutChanged();
	emit itemLoaded( createIndex(item->row(), 0, item) );
}

void ServersModel::refresh(Item* item)
{
	qDeleteAll(item->children);
	item->children.clear();
	qDeleteAll(item->files);
	item->files.clear();

	reset();

	loadItem(item);
}

void ServersModel::changeSettings(QString ftpHost, int ftpPort, bool ftpPassiveMode, QString ftpLogin, QString ftpPassword, QString ftpBaseDir)
{
//	clear();
//	ftpData->changeSettings(ftpHost, ftpPort, ftpPassiveMode, ftpLogin, ftpPassword, ftpBaseDir);
//	updateModel();
}

void ServersModel::loadItem(Item* item)
{
	//clear();

	if( item->children.size() > 0 || item->files.size() > 0 )
	{
		item->server->sendTechSpecUrl(item);
		allPartsDownloaded(item);
		return;
	}

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

void ServersModel::abort()
{
	foreach(BaseDataSource *s, servers)
	{
		s->abort();
	}
}
