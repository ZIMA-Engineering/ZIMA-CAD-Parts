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
	void loadRootItem(Item *item);
	void loadDirectory(Item* item);
	void changeSettings(QString remoteHost, int remotePort, bool ftpPassiveMode, QString remoteLogin, QString remotePassword, QString remoteBaseDir);
	void reset();
	void deleteFiles(QList<File*> files);
	void addFileToDownload(File *f);
	void downloadFiles(QList<File*> files, QString dir);
	void downloadFile(File* file);
	void resumeDownload();
	void abort();
	void deleteDownloadQueue();
	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings);
	void assignTechSpecUrlToItem(QString url, Item *item, QString lang, bool overwrite = false);
	void assignPartsIndexUrlToItem(QString url, Item *item, QString lang, bool overwrite = false);

protected:
	void checkAndSendTechSpecUrl(Item *item);

private slots:
	void ftpListInfo(const QUrlInfo&);
	void ftpCommandFinished(int, bool);
	void ftpStateChanged(int);
	void ftpDataTransferProgress(qint64 done, qint64 total);
	void ftpFileDownloadFinished(int, bool);

private:
	void checkConnection(QFtp *f);
	void ftpListItemInQueue();
	void checkLoadedItem();

//	QList<QString>  paramList;
	QList<Item*> loadItemQueue;

	Item            *ftpCurrentItem; //the 'directory part' of the current (while listing) directory
	QString         ftpCurrentDir;
	QList<Item*>    dirsToList; //list of subdirectories that need to be searched and listed
	int             ftpListId; //the command id used to identify the listing command in ftpCommandDone
//	int             dbFilesQueued; //number of .db files for which active ftp tasks exist
	int browseDepth;
	bool hasTechSpecDir;
	int techSpecListId;
	bool hasMetadata;
	bool metadataChanged;
	bool hasLogo;
	bool hasLogoText;
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
};

#endif // FTPDATASOURCE_H
