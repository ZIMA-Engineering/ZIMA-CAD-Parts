#include "serversmodel.h"
#include <QDebug>

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
		if( item->isServer )
			return serverIcon;
		else if( item->isDir )
			return dirIcon;
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

void ServersModel::setServerData(QVector<FtpServer*> srv)
{
	//qDeleteAll(rootItem->children);
	rootItem->children.clear();
	servers = srv;

	foreach(FtpServer *s, servers)
	{
		Item *i = new Item();
		i->name = s->address;
		i->isServer = true;
		i->server = s;
		i->server->ftpData = new FtpData();
		i->server->ftpData->rootItem = i;
		i->server->ftpData->changeSettings(i->server->address,
					i->server->port,
					i->server->passiveMode,
					i->server->login,
					i->server->password,
					i->server->baseDir);
		connect(i->server->ftpData, SIGNAL(allPartsDownloaded(Item*)), this, SLOT(allPartsDownloaded(Item*)));
		connect(i->server->ftpData, SIGNAL(techSpecAvailable(QUrl)), this, SIGNAL(techSpecAvailable(QUrl)));
		connect(i->server->ftpData, SIGNAL(statusUpdated(QString)), this, SIGNAL(statusUpdated(QString)));
		connect(i->server->ftpData, SIGNAL(filesDownloaded()), this, SIGNAL(filesDownloaded()));
		connect(i->server->ftpData, SIGNAL(errorOccured(QString)), this, SIGNAL(errorOccured(QString)));

		i->parent = rootItem;
		i->path = i->server->baseDir.endsWith('/') ? i->server->baseDir : i->server->baseDir + "/";

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
	item->children.clear();
	item->files.clear();

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
		item->server->ftpData->sendTechSpecUrl(item);
		allPartsDownloaded(item);
		return;
	}

	item->server->ftpData->changeSettings(item->server->address,
				item->server->port,
				item->server->passiveMode,
				item->server->login,
				item->server->password,
				item->server->baseDir);
	item->server->ftpData->loadDirectory(item);
}

void ServersModel::clear()
{
	foreach(FtpServer *s, servers)
	{
		s->ftpData->reset();
	}
	//reset(); // ???
}

void ServersModel::abort()
{
	foreach(FtpServer *s, servers)
	{
		s->ftpData->abort();
	}
}
