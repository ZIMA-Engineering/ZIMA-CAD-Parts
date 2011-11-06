#ifndef BASEREMOTEDATASOURCE_H
#define BASEREMOTEDATASOURCE_H

#include "basedatasource.h"

class Item;
struct File;

class BaseRemoteDataSource : public BaseDataSource
{
	Q_OBJECT
public:
	virtual QString internalName() = 0;
	virtual QIcon itemIcon(Item *item);
	virtual QIcon dataSourceIcon();

	// FIXME: make protected
	QString remoteHost, remoteLogin, remotePassword, remoteBaseDir;
	int     remotePort;
public slots:
	void sendTechSpecUrl(Item* item);
	virtual void loadDirectory(Item* item) = 0;
	virtual void downloadFiles(QList<File*> files, QString dir) = 0;
	virtual void downloadFile(File* file) = 0;
	virtual void abort() = 0;
	virtual void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings);
protected:
	QString cacheDirPath();
};

#endif // BASEREMOTEDATASOURCE_H
