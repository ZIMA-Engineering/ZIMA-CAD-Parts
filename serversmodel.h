#ifndef SERVERSMODEL_H
#define SERVERSMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QVector>
#include <QKeyEvent>
#include "ftpdata.h"
#include "ftpserver.h"
#include "item.h"

struct ServerItem
{
	ServerItem()
	{
		parent = 0;
		isServer = false;
	}

	~ServerItem()
	{
		qDeleteAll(children);
	}

	int row() const
	{
		if (parent)
			return parent->children.indexOf(const_cast<ServerItem*>(this));
		return 0;
	}

	ServerItem *child(int r)
	{
		return children.value(r);
	}

	QString name;
	bool    isServer;


	ServerItem          *parent;
	QList<ServerItem*>  children;
};

class ServersModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	ServersModel(QObject *parent = 0);
	~ServersModel();

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
	void changeSettings(QString ftpHost, int ftpPort, bool ftpPassiveMode, QString ftpLogin, QString ftpPassword, QString ftpBaseDir);
	void setIcons(QIcon dir, QIcon server);
	void setServerData(QVector<FtpServer*>);
public slots:
	//    void updateModel();
	//    void clear();
	//    void downloadSelected(QString targetDir, Part *parent = 0);
	void refresh(Item* item);
	void clear();
	void loadItem(Item *item);
	void abort();
protected:
	//FtpData *ftpData;
protected slots:
	void allPartsDownloaded(Item* item);
private:
	QIcon dirIcon, serverIcon;
	QVector<FtpServer*> servers;
	Item *rootItem;
signals:
	void itemLoaded(const QModelIndex&);
	void techSpecAvailable(QUrl);
	void statusUpdated(QString);
	void errorOccured(QString);
	void filesDownloaded();
};

#endif // SERVERSMODEL_H
