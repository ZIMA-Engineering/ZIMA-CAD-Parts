#ifndef FTPDATA_H
#define FTPDATA_H

#include <QObject>
#include <QString>
#include <QFtp>
#include <QList>
#include <QStringList>
#include <QSettings>
#include <QUrl>
#include "item.h"

#define TECHSPEC_DIR "0000-index"

class Item;
class File;

class FtpData : public QObject
{
	Q_OBJECT
public:
	FtpData(QObject *parent = 0);
	~FtpData();

	Item *getRootItem();
	QList<QString> *getParamList();

	Item    *rootItem;
public slots:
	//    void loadDirectory(QString dir);
	void loadDirectory(Item* item);
	void loadFile(Item *p, QSettings &settings);
	int downloadPart(QString targetDir, Item*);
	void changeSettings(QString ftpHost, int ftpPort, bool ftpPassiveMode, QString ftpLogin, QString ftpPassword, QString ftpBaseDir);
	void reset();
	void sendTechSpecUrl(Item* item);
	void downloadFiles(QList<File*> files, QString dir);
	void downloadFile(File* file);
	void abort();

private slots:
	void ftpListInfo(const QUrlInfo&);
	//void techSpecListInfo(const QUrlInfo&);
	void ftpCommandFinished(int, bool);
	void ftpStateChanged(int);

private:

	QList<QString>  paramList;

	//settings
	QString ftpHost, ftpLogin, ftpPassword, ftpBaseDir;
	int     ftpPort;
	bool    ftpPassiveMode;

	Item            *ftpCurrentItem; //the 'directory part' of the current (while listing) directory
	QString         ftpCurrentDir;
	QList<Item*>    dirsToList; //list of subdirectories that need to be searched and listed
	int             ftpListId; //the command id used to identify the listing command in ftpCommandDone
	int             dbFilesQueued; //number of .db files for which active ftp tasks exist
	bool hasTechSpecDir;
	int techSpecListId;
	QList<File*> thumbnails;

	QMap<int, QFile*> techSpecFiles;
	QMap<int, Item*>    partTasks; //maps ftp task indexes to parts for .db file transfer
	QMap<int, File*>    partPicTasks; //the same thing as above, but for picture file transfer
	QMap<int, File*>   fileTasks; //for actual files file transfer
	QList<File*> filesToDownload;
	QString targetDir;
	QStringList techSpecIndexes;

	QFtp    *ftp;

signals:
	void dirListingDone();
	void partDownloaded(Item*);
	void updateAvailable();
	void allPartsDownloaded(Item*);
	void filesDownloaded(); //TODO: identify this somehow
	void gotPicture(Item*);
	void statusUpdated(QString);
	void techSpecAvailable(QUrl);
	void errorOccured(QString);
};

#endif // FTPDATA_H
