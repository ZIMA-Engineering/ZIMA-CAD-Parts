#ifndef BASEDATASOURCE_H
#define BASEDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QSettings>
#include <QIcon>
#include <QApplication>

#include "item.h"

#define TECHSPEC_DIR "0000-index"

class QListWidgetItem;
class Item;
struct File;

enum DataSources {
	LOCAL=0,
	FTP,
	UNDEFINED
};

class BaseDataSource : public QObject
{
	Q_OBJECT
public:
	explicit BaseDataSource(QObject *parent = 0);
	Item *getRootItem();
	virtual QString internalName() = 0;
	virtual QIcon itemIcon(Item *item);
	virtual QIcon dataSourceIcon();

	Item    *rootItem;
	QListWidgetItem *lwItem;
	QString label;
	DataSources dataSource;
public slots:
	virtual void loadDirectory(Item* item) = 0;
	virtual void sendTechSpecUrl(Item* item) = 0;
	virtual void downloadFiles(QList<File*> files, QString dir) = 0;
	virtual void downloadFile(File* file) = 0;
	virtual void resumeDownload() = 0;
	virtual void abort() = 0;
	virtual void loadSettings(QSettings& settings) = 0;
	virtual void deleteDownloadQueue();
	void saveSettings(QSettings& settings);
protected:
	QStringList techSpecIndexes;
signals:
	void partDownloaded(Item*);
	void updateAvailable();
	void allPartsDownloaded(Item*);
	void fileProgress(File*);
	void fileDownloaded(File*);
	void filesDownloaded(); //TODO: identify this somehow
	void gotThumbnail(File*);
	void statusUpdated(QString);
	void techSpecAvailable(QUrl);
	void errorOccured(QString);
};

#endif // BASEDATASOURCE_H
