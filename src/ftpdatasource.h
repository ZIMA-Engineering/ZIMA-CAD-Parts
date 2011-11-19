#ifndef FTPDATASOURCE_H
#define FTPDATASOURCE_H

#include <QString>
#include <QFtp>
#include <QList>
#include <QStringList>
#include <QSettings>
#include <QUrl>

#include "baseremotedatasource.h"

struct File;

class FtpDataSource : public BaseRemoteDataSource
{
	Q_OBJECT
public:
	FtpDataSource();
	~FtpDataSource();
	QString internalName();

	// FIXME: make private
	bool    ftpPassiveMode;
public slots:
	//    void loadDirectory(QString dir);
	void loadDirectory(Item* item);
	void changeSettings(QString remoteHost, int remotePort, bool ftpPassiveMode, QString remoteLogin, QString remotePassword, QString remoteBaseDir);
	void reset();
	void addFileToDownload(File *f);
	void downloadFiles(QList<File*> files, QString dir);
	void downloadFile(File* file);
	void resumeDownload();
	void abort();
	void deleteDownloadQueue();
	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings);
private slots:
	void ftpListInfo(const QUrlInfo&);
	void ftpCommandFinished(int, bool);
	void ftpStateChanged(int);
	void ftpDataTransferProgress(qint64 done, qint64 total);
	void ftpFileDownloadFinished(int, bool);
private:
	QList<QString>  paramList;

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
	QFtp *dlFtp;

	void checkConnection(QFtp *f);
};

#endif // FTPDATASOURCE_H
