#ifndef SERVERSMODEL_H
#define SERVERSMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QVector>
#include <QKeyEvent>
#include "basedatasource.h"
#include "item.h"

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
	void setServerData(QVector<BaseDataSource*>);
public slots:
	//    void updateModel();
	//    void clear();
	//    void downloadSelected(QString targetDir, Part *parent = 0);
	void refresh(Item* item);
	void clear();
	void loadItem(Item *item);
	void downloadFiles(QString dir);
	void resumeDownload();
	void uncheckAll(Item *item = 0);
	void deleteDownloadQueue();
	void saveQueue(QSettings *settings);
	int loadQueue(QSettings *settings);
	void abort();
protected:
	//FtpData *ftpData;
	QList<File*> getCheckedFiles(Item *item);
protected slots:
	void allPartsDownloaded(Item* item);
private:
	QIcon dirIcon, serverIcon;
	QVector<BaseDataSource*> servers;
	Item *rootItem;
	QList<File*> downloadQueue;

private slots:
	void dataSourceFinishedDownloading();
signals:
	void itemLoaded(const QModelIndex&);
	void techSpecAvailable(QUrl);
	void statusUpdated(QString);
	void errorOccured(QString);
	void fileProgress(File*);
	void fileDownloaded(File*);
	void filesDownloaded();
	void newDownloadQueue(QList<File*>*);
	void queueChanged();
};

#endif // SERVERSMODEL_H
