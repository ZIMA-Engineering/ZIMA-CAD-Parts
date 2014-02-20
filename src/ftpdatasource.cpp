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

#include "ftpdatasource.h"
#include "metadata.h"

#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <QSettings>

FtpDataSource::FtpDataSource() :
	BaseRemoteDataSource()
{
	ftpListId = -1;
	dataSource = FTP;

	remoteHost = "localhost";
	remotePort = 21;
	remoteBaseDir = "/";
	ftpPassiveMode = true;

	ftp = new QFtp(this);
	dlFtp = new QFtp(this);

	connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(ftpListInfo(QUrlInfo)));
	connect(ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpCommandFinished(int, bool)));
	//connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpStateChanged(int)));
	connect(dlFtp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(ftpDataTransferProgress(qint64,qint64)));
	connect(dlFtp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpFileDownloadFinished(int,bool)));
	connect(dlFtp, SIGNAL(stateChanged(int)), this, SLOT(ftpStateChanged(int)));

	reset();
}

FtpDataSource::~FtpDataSource()
{
	if (ftp)
	{
		ftp->abort();
		ftp->deleteLater();
	}
	//delete rootItem;
}

QString FtpDataSource::internalName()
{
	return "ftp";
}

void FtpDataSource::reset()
{
//	if (ftp)
//	{
//		ftp->abort();
//		ftp->deleteLater();
//		ftp = 0;
//	}
	ftpListId = -1;

	if(rootItem)
	{
		// FIXME
		//delete rootItem;
		//rootItem = 0;
	}

	partPicTasks.clear();
	partTasks.clear();
	fileTasks.clear();
	dirsToList.clear();
	//paramList.clear();
	thumbnails.clear();
	techSpecFiles.clear();

	hasTechSpecDir = false;
	techSpecListId = -1;
	hasMetadata = false;
}

void FtpDataSource::loadRootItem(Item *item)
{
	loadItemLogo(item);
}

void FtpDataSource::checkAndSendTechSpecUrl(Item *item)
{
	if(techSpecItem != item)
		return;

	BaseRemoteDataSource::checkAndSendTechSpecUrl(item);
}

void FtpDataSource::loadDirectory(Item* item)
{
	if(loadItemQueue.contains(item))
	{
		//qDebug() << "Item" << item->path << "already in queue";
		return;
	}

	loadItemQueue << item;

	if(ftpListId == -1)
		ftpListItemInQueue();
}

void FtpDataSource::ftpStateChanged(int state)
{
	switch( state )
	{
	case QFtp::HostLookup:
		emit statusUpdated(tr("Resolving %1...").arg(remoteHost));
		break;
	case QFtp::Connecting:
		emit statusUpdated(tr("Connecting..."));
		break;
	case QFtp::Connected:
		emit statusUpdated(tr("Connected."));
		break;
	case QFtp::LoggedIn:
		emit statusUpdated(tr("Logging in..."));
		break;
	case QFtp::Closing:
		emit statusUpdated(tr("Disconnecting..."));
		break;
	}
}

void FtpDataSource::ftpDataTransferProgress(qint64 done, qint64 total)
{
	Q_UNUSED(total);

	if( !fileTasks.contains( dlFtp->currentId() ) )
		return;

	File *f = fileTasks[ dlFtp->currentId() ];

	f->bytesDone = done;

	emit fileProgress(f);
}

void FtpDataSource::changeSettings(QString remoteHost, int remotePort, bool ftpPassiveMode, QString remoteLogin, QString remotePassword, QString remoteBaseDir)
{
	this->remoteHost = remoteHost;
	this->remotePort = remotePort;
	this->ftpPassiveMode = ftpPassiveMode;
	this->remoteLogin = remoteLogin;
	this->remotePassword = remotePassword;
	this->remoteBaseDir = remoteBaseDir;
}

void FtpDataSource::ftpListItemInQueue()
{
	if(ftpListId != -1 && !dirsToList.isEmpty() && ftpCurrentItem == dirsToList.first())
	{
		//qDebug() << "removing" << dirsToList.first()->path << "from dirstolist";
		emit itemLoaded(dirsToList.first());

		dirsToList.removeFirst();
	}

	if(dirsToList.isEmpty())
	{
		if(ftpListId != -1 && !loadItemQueue.isEmpty())
		{
			loadItemQueue.first()->hasLoadedChildren = true;
			//qDebug() << "removing" << loadItemQueue.first()->name << "from loadItemqueue";

			emit itemLoaded(loadItemQueue.first());

			loadItemQueue.removeFirst();
		}

		if(loadItemQueue.isEmpty())
		{
			ftpListId = -1;

			emit allItemsLoaded();

			return;
		}

		Item *item = loadItemQueue.first();

		checkConnection(ftp);

		if(!item->hasLoadedChildren && !item->children.isEmpty())
		{
			dirsToList << item->children;
			//dirsToList.removeFirst();

			remoteBaseDir = item->children.first()->path;
			ftpCurrentItem = item->children.first();
			ftpCurrentItem->hasTechSpecs = false;

			hasTechSpecDir = false;
			hasMetadata = false;
			metadataChanged = false;
			techSpecFilesUpdated = false;
			hasLogo = false;
			hasLogoText = false;
			qDeleteAll(thumbnails);
			thumbnails.clear();

			qDebug() << "Loading children of" << item->path << "dirsToList size=" << dirsToList.count();
			//emit statusUpdated(tr("Loading %1...").arg(item->getLabel()));

			emit loadingItem(item);

			ftpListId = ftp->list(item->children.first()->path);

//			emit itemLoaded(dirsToList.first());

			return;
		}

		if(!item->files.isEmpty())
		{
			qDebug() << "Stop, this item has files ;)" << item->name;
			//sendTechSpecUrl(item);
			checkAndSendTechSpecUrl(item);
			emit itemLoaded(item);

			loadItemQueue.removeFirst();

			ftpListItemInQueue();

			return;
		}

		// Start loading another item in queue
		remoteBaseDir = item->path;

		qDebug() << "Loading directory " << remoteBaseDir;

		emit loadingItem(item);

		//emit statusUpdated(tr("Entering ") + remoteBaseDir);

		ftpCurrentItem = item;
		ftpCurrentDir = remoteBaseDir;

		ftpCurrentItem->hasTechSpecs = false;

		browseDepth = 2;
		hasTechSpecDir = false;
		hasMetadata = false;
		metadataChanged = false;
		techSpecFilesUpdated = false;
		hasLogo = false;
		hasLogoText = false;
		qDeleteAll(thumbnails);
		thumbnails.clear();

		ftpListId = ftp->list(remoteBaseDir);
	} else {
		// Start browsing another directory in item
		Item *item = dirsToList.first();
		//dirsToList.removeFirst();

		remoteBaseDir = item->path;

		checkConnection(ftp);

		qDebug() << "Browsing" << item->path << "dirsToList size=" << dirsToList.count();

		ftpCurrentItem = item;
		ftpCurrentItem->hasTechSpecs = false;

		hasTechSpecDir = false;
		hasMetadata = false;
		metadataChanged = false;
		hasLogo = false;
		hasLogoText = false;
		qDeleteAll(thumbnails);
		thumbnails.clear();

		ftpListId = ftp->list(item->path);
	}
}

void FtpDataSource::ftpListInfo(const QUrlInfo &info)
{
	QString n = info.name();

	if( techSpecListId > -1 )
	{
		if( !info.isFile() )
			return;

		QString filePath = cacheDirPath() + "/" + remoteHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n;
		QFileInfo fi(filePath);
		QDir dir;

		// IMPORTANT: Here we assume that FTP server returns the date in UTC
		QDateTime remoteFileTime = info.lastModified();
		remoteFileTime.setTimeSpec(Qt::UTC);

		if( !dir.exists(filePath) || remoteFileTime > fi.lastModified().toUTC() )
		{
			QFile* f = new QFile(filePath);
			f->open(QFile::WriteOnly);

			techSpecFiles[ ftp->get(remoteBaseDir + "/" + TECHSPEC_DIR + "/" + n, f) ] = f;
			//qDebug() << "Queue techspec file" << ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n;

			if(n == METADATA_FILE)
				metadataChanged = true;

			if(ftpCurrentItem == techSpecItem)
				techSpecFilesUpdated = true;
		}

		if( n == METADATA_FILE )
			hasMetadata = true;
		else if(n == LOGO_FILE)
			hasLogo = true;
		else if(n == LOGO_TEXT_FILE)
			hasLogoText = true;

		return;
	}

	if (info.isDir())
	{
		if( n == TECHSPEC_DIR )
		{
			//qDebug() << "Hey, I have found TECHSPEC directory... path is" << (ftpCurrentItem->path + n + "/") << "last edit is" << info.lastModified();
			hasTechSpecDir = true;
			ftpCurrentItem->hasTechSpecs = true;
		} else {
			Item *i = new Item();
			i->setType(Item::Dir);
			i->name = n;
			i->parent = ftpCurrentItem;
			i->path = remoteBaseDir + n + "/";
			i->server = rootItem->server;

			loadItemLogo(i);

			QString metadata = cacheDirPath() + "/" + remoteHost + "/" + i->path + "/" + TECHSPEC_DIR + "/" + METADATA_FILE;

			if(QFile::exists(metadata))
				createMetadata(i);

			ftpCurrentItem->children.append(i);

			// We will list only one level from the parent, since the only thing we want is 0000-index with it's metadata
			// FIXME: We should be able to set depth, or list whole subtree
			if(!loadItemQueue.isEmpty() && ftpCurrentItem == loadItemQueue.first())
			{
				dirsToList << i;
				//qDebug() << "new directory" << i->path << "IN QUEUE";
			} else {
				;//qDebug() << "new directory" << i->path << "NOT IN QUEUE";
			}
			emit itemInserted(i);
		}

		//qDebug() << "reading directory" << n;
	}
	else if( info.isFile() ) {
		File *f = new File;
		//f->isDir = false;
		f->setName(n);
		f->size = info.size();
		f->lastModified = info.lastModified();
		f->lastModified.setTimeSpec(Qt::UTC);
		f->parentItem = ftpCurrentItem;

		if( n.endsWith(".png", Qt::CaseInsensitive) || n.endsWith(".jpg", Qt::CaseInsensitive) )
		{
			thumbnails << f;
			return;
		}

		f->path = remoteBaseDir + n;

		//f->parent = ftpCurrentItem;
		//f->server = rootItem->server;

		ftpCurrentItem->files.append(f);
	}
}

void FtpDataSource::ftpCommandFinished(int id, bool error)
{
	if (error)
	{
		//emit statusUpdated( tr("FTP error: %1").arg(ftp->errorString()) );
		emit errorOccured(ftp->errorString());
		qDebug() << "FTP error: " << ftp->errorString();
	} else if (id == ftpListId) { //finished listing a directory
		//ftpListId = -1;

//		Item *currentDir = 0;

		if( hasTechSpecDir )
		{
			//qDebug() << "Tech spec list for" << remoteBaseDir + "/" + TECHSPEC_DIR;

			ftp->cd(remoteBaseDir + "/" + TECHSPEC_DIR);
			techSpecListId = ftp->list();

			QDir dir;
			dir.mkpath( cacheDirPath() + "/" + remoteHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR );
		}

		// Check & download thumbnails
		// Make sure this is done before calling ftpListItemInQueue(), since all thumbnails MUST be in download queue before another dir listing
		/*foreach(File* file, ftpCurrentItem->files)
		{
			for(int i = 0; i < thumbnails.size(); i++)
			{
				//qDebug() << "file " << file->name.section('.', 0, 0) << "checking for thumbnail in" << thumbnails[i].section('.', -2, -2);
				if( file->name.section('.', 0, 0) == thumbnails[i]->name.section('.', -2, -2) )
				{
					QString thumbPath = cacheDirPath() + "/" + remoteHost + "/" + ftpCurrentItem->path + "/" + thumbnails[i]->name;
					QFileInfo fi(thumbPath);

					if( !QFile::exists(thumbPath) || thumbnails[i]->lastModified > fi.lastModified().toUTC() )
					{
						file->pixmapPath = thumbPath;

						if(thumbnails[i]->openFtpFile)
							break;

						QDir dir;
						dir.mkpath(cacheDirPath() + "/" + remoteHost + "/" + ftpCurrentItem->path);

						QFile *f = new QFile(thumbPath);
						f->open(QFile::WriteOnly);

						thumbnails[i]->openFtpFile = f;
						thumbnails[i]->pixmapPath = thumbPath;

						//qDebug() << "Downloading pixmap " << QString(ftpCurrentItem->path + "/" + thumbnails[i]->name) << "and saving to" << thumbPath;
						partPicTasks[ ftp->get(remoteBaseDir + "/" + thumbnails[i]->name, f) ] = thumbnails[i];

						//thumbnails.removeAt(i--);
					} else {
						file->pixmapPath = thumbPath;
						file->pixmap = QPixmap(thumbPath).scaledToWidth(100);
					}
					break;
				}

			}
		}*/

		foreach(File *thumb, thumbnails)
		{
			Thumbnail *t = new Thumbnail(ftpCurrentItem, thumb->name);
			ftpCurrentItem->addThumbnail(t);

			QFileInfo fi(t->absolutePath());

			if( !QFile::exists(fi.absoluteFilePath()) || thumb->lastModified > fi.lastModified().toUTC() )
			{
				if(thumb->openFtpFile)
					break;

				thumb->thumbnail = t;
				t->setReady(false);

				QDir dir;
				dir.mkpath(cacheDirPath() + "/" + remoteHost + "/" + ftpCurrentItem->path);

				QFile *f = new QFile(t->absolutePath());
				f->open(QFile::WriteOnly);

				thumb->openFtpFile = f;

				partPicTasks[ ftp->get(remoteBaseDir + "/" + thumb->name, f) ] = thumb;
			}
		}

		assignThumbnailsToFiles(ftpCurrentItem);
		determineFileVersions(ftpCurrentItem);

		// Start browsing next folder in dirsToList
		if(!hasTechSpecDir && partPicTasks.isEmpty())
			ftpListItemInQueue();

//		if(!loadItemQueue.isEmpty() && ftpCurrentItem == loadItemQueue.first())
//		{
//			loadItemQueue.first()->hasLoadedChildren = true;
//			//qDebug() << "removing" << loadItemQueue.first()->name << "from loadItemqueue";
//			loadItemQueue.removeFirst();

//			emit itemLoaded(currentDir ? currentDir : ftpCurrentItem);
//		}

		//start listing another one
//		Item *p = dirsToList.front();
//		dirsToList.pop_front();
//		ftpCurrentDir = p->path + (p->path == "/" ? "" : "/") + p->name;
//		ftpCurrentDirPart = p;
//		while (ftpCurrentDir.indexOf("//") >= 0)
//			ftpCurrentDir.replace("//", "/");
//		emit statusUpdated("Browsing " + ftpCurrentDir);
//		qDebug() << "Browsing " << ftpCurrentDir;

//		ftpListId = ftp->list(ftpCurrentDir);
	} else if( id == techSpecListId ) {
		techSpecListId = -1;

		if( techSpecFiles.isEmpty() )
		{
			checkLoadedItem();

			if(partPicTasks.isEmpty())
				// Start browsing next folder in dirsToList
				ftpListItemInQueue();
		}



	} else if( techSpecFiles.contains(id) ) {
		QFile* f = techSpecFiles[id];

		//qDebug() << "Techspec file" << f->fileName() << "downloaded";

		f->close();

		techSpecFiles.remove(id);

		if( techSpecFiles.isEmpty() )
		{
			checkLoadedItem();

			if(partPicTasks.isEmpty())
				// Start browsing next folder in dirsToList
				ftpListItemInQueue();
		}
	} else if (partPicTasks.contains(id)) { //finished downloading a picture
		File *thumb = partPicTasks[id];
		partPicTasks.remove(id);

		if (thumb->openFtpFile)
			thumb->openFtpFile->close();
		delete thumb->openFtpFile;
		thumb->openFtpFile = 0;

		if (!error)
		{
			foreach(File *f, thumb->parentItem->files)
			{
				if(f->thumbnail == thumb->thumbnail)
					emit thumbnailLoaded(f);
			}
		}

		if(partPicTasks.isEmpty() && techSpecFiles.isEmpty())
			ftpListItemInQueue();
	}/* else if (partTasks.contains(id)) //finished downloading a .db file
	{
		dbFilesQueued--;
		if (!error)
		{
			//create a temporary file to load data from the .db file from server
			QTemporaryFile tf;
			tf.open();
			tf.write(ftp->readAll());
			tf.seek(0);
			QSettings settings(tf.fileName(), QSettings::IniFormat);
			tf.close();

			Item *p = partTasks[id];
			partTasks.remove(id);

			p->setNotEmpty();
//			loadFile(p, settings);

			emit statusUpdated("Got " + p->name);
			qDebug() << "Got " << p->name;
			emit partDownloaded(p);
		}

		if (ftpListId == -1 && dbFilesQueued <= 0)
		{
			qDebug() << "All done";
			//emit allPartsDownloaded();
		}

	} */else if (fileTasks.contains(id)) //actual file downloaded
	{

		//else
		//	downloadFile(filesToDownload.first());
	}
}

void FtpDataSource::checkLoadedItem()
{
	if( hasMetadata )
	{
		//qDebug() << "Metadata loaded (tech spec files)";

		if(ftpCurrentItem->metadata)
		{
			if(metadataChanged)
				ftpCurrentItem->metadata->refresh();
		} else {
			createMetadata(ftpCurrentItem);
			qDebug() << "Metadata loaded - techspecfiles" << ftpCurrentItem->metadata->getLabel();
		}

	} else if( ftpCurrentItem->metadata )
	{
		delete ftpCurrentItem->metadata;
		ftpCurrentItem->metadata = 0;
	}

	if(hasLogo || hasLogoText)
	{
		ftpCurrentItem->logo = QPixmap(cacheDirPath() + "/" + remoteHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + (hasLogo ? LOGO_FILE : LOGO_TEXT_FILE));
		ftpCurrentItem->showText = hasLogoText;

		emit updateAvailable(ftpCurrentItem);
	}

	if(!loadItemQueue.isEmpty() && ftpCurrentItem == loadItemQueue.first())
		//sendTechSpecUrl(ftpCurrentItem);
		checkAndSendTechSpecUrl(ftpCurrentItem);
}

void FtpDataSource::ftpFileDownloadFinished(int id, bool error)
{
	if (error)
	{
		emit errorOccured(ftp->errorString());
		qDebug() << "FTP error: " << ftp->errorString();
		qDebug() << "I am " << label << remoteHost << remoteLogin;
	} else if( fileTasks.contains(id) ) {
		qDebug() << "Finished downloading " << fileTasks[id]->name;
		emit statusUpdated("Finished downloading " + fileTasks[id]->name);

		File *file = fileTasks[id];

		QFile *f = file->openFtpFile;

		delete f;
		file->openFtpFile = 0;
		file->targetPath.clear();

		filesToDownload.removeFirst();
		fileTasks.remove(id);

		emit fileDownloaded(file);

		file->bytesDone = 0;

		if( filesToDownload.isEmpty() )
			emit filesDownloaded();
	}
}

void FtpDataSource::deleteFiles(QList<File*> files)
{
	foreach(File *f, files)
	emit fileError(BaseDataSource::Delete, new BaseDataSource::Error(f, tr("Operation not supported")));

	emit filesDeleted();
}

void FtpDataSource::addFileToDownload(File *f)
{
	filesToDownload << f;
}

void FtpDataSource::downloadFiles(QList<File*> files, QString dir)
{
	targetDir = dir;
	filesToDownload << files;

	checkConnection(dlFtp);

	//if( filesToDownload.size() > 0 )
	//	downloadFile(filesToDownload[0]);
	foreach(File *f, filesToDownload)
	downloadFile(f);
}

void FtpDataSource::downloadFile(File* file)
{
	emit statusUpdated(tr("Downloading ") + file->path);

	qDebug() << "Downloading" << file->name;

	if( file->targetPath.isEmpty() )
		file->targetPath = targetDir + "/" + file->name;

	file->openFtpFile = new QFile(file->targetPath);
	file->openFtpFile->open(QFile::WriteOnly);
	fileTasks[ dlFtp->get(file->path, file->openFtpFile) ] = file;
}

void FtpDataSource::resumeDownload()
{
	if( !filesToDownload.count() )
		return;
	qDebug() << "Resuming download";
	checkConnection(dlFtp);

	foreach(File *f, filesToDownload)
	downloadFile(f);
}

void FtpDataSource::deleteDownloadQueue()
{
	abort();
	filesToDownload.clear();
	fileTasks.clear();
}

void FtpDataSource::abort()
{
	if( ftp->state() != QFtp::Unconnected )
		ftp->abort();
	if( dlFtp->state() != QFtp::Unconnected )
		dlFtp->abort();
	fileTasks.clear();
	loadItemQueue.clear();
	dirsToList.clear();
	techSpecFiles.clear();
	partPicTasks.clear();

	ftpListId = -1;
	techSpecListId = -1;
}

void FtpDataSource::loadSettings(QSettings &settings)
{
	BaseRemoteDataSource::loadSettings(settings);

	ftpPassiveMode = settings.value("PassiveMode", true).toBool();
}

void FtpDataSource::saveSettings(QSettings &settings)
{
	BaseRemoteDataSource::saveSettings(settings);

	settings.setValue("PassiveMode", ftpPassiveMode);
}

void FtpDataSource::assignTechSpecUrlToItem(QString url, Item *item, QString lang, bool overwrite)
{
	QByteArray htmlIndex = QString("<html>\n"
	                               "	<head>\n"
	                               "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
	                               "	</head>\n"
	                               "</html>\n").arg(url).toUtf8();
	QString targetFile = item->path + "/" + TECHSPEC_DIR + "/" + "index_" + lang + ".html";

	QDir cachedTechSpecDir = (cacheDirPath() + "/" + remoteHost + "/" + item->path + "/" + TECHSPEC_DIR);

	if(!cachedTechSpecDir.exists())
		cachedTechSpecDir.mkdir(cachedTechSpecDir.absolutePath());

	QFile cachedIndexFile(cachedTechSpecDir.absoluteFilePath("index_" + lang + ".html"));

	if(cachedIndexFile.exists() && !overwrite)
	{
		emit techSpecsIndexAlreadyExists(item);
		return;
	}

	if(!cachedIndexFile.open(QIODevice::WriteOnly))
		return; // FIXME: Notify user on failure?

	cachedIndexFile.write(htmlIndex);
	cachedIndexFile.close();

	if(!item->hasTechSpecs)
		ftp->mkdir(item->path + "/" + TECHSPEC_DIR);

	ftp->put(htmlIndex, targetFile);
}

void FtpDataSource::assignPartsIndexUrlToItem(QString url, Item *item, QString lang, bool overwrite)
{
	QByteArray htmlIndex = QString("<html>\n"
	                               "	<head>\n"
	                               "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
	                               "	</head>\n"
	                               "</html>\n").arg(url).toUtf8();
	QString targetFile = item->path + "/" + TECHSPEC_DIR + "/" + "index-parts_" + lang + ".html";

	QDir cachedTechSpecDir = (cacheDirPath() + "/" + remoteHost + "/" + item->path + "/" + TECHSPEC_DIR);

	if(!cachedTechSpecDir.exists())
		cachedTechSpecDir.mkdir(cachedTechSpecDir.absolutePath());

	QFile cachedIndexFile(cachedTechSpecDir.absoluteFilePath("index-parts_" + lang + ".html"));

	if(cachedIndexFile.exists() && !overwrite)
	{
		emit partsIndexAlreadyExists(item);
		return;
	}

	if(!cachedIndexFile.open(QIODevice::WriteOnly))
		return; // FIXME: Notify user on failure?

	cachedIndexFile.write(htmlIndex);
	cachedIndexFile.close();

	if(!item->hasTechSpecs)
		ftp->mkdir(item->path + "/" + TECHSPEC_DIR);

	ftp->put(htmlIndex, targetFile);
}

void FtpDataSource::checkConnection(QFtp *f)
{
	if( f->state() == QFtp::Unconnected )
	{
		if( !ftpPassiveMode )
			f->setTransferMode(QFtp::Active);

		emit statusUpdated(tr("Connecting to ") + remoteHost);
		f->connectToHost(remoteHost, remotePort);

		if (!remoteLogin.isEmpty())
			f->login(remoteLogin, remotePassword);
		else
			f->login();
	}
}
