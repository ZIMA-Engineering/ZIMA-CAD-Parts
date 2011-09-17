#include "ftpdata.h"
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <QSettings>

//TODO: somehow find out when all of the .db files are finished downloading
//also some state management (in progress, cancelation, waiting, etc.)

FtpData::FtpData(QObject *parent) : QObject(parent)
{
	rootItem = 0;
	ftpListId = -1;

	ftp = new QFtp(this);

	connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(ftpListInfo(QUrlInfo)));
	connect(ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpCommandFinished(int, bool)));
	connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpStateChanged(int)));

	techSpecIndexes << "index.html" << "index.htm";

	reset();
}

FtpData::~FtpData()
{
	if (ftp)
	{
		ftp->abort();
		ftp->deleteLater();
	}
	//delete rootItem;
}

void FtpData::reset()
{
	qDebug() << "Resetting data in model";
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
	paramList.clear();
	thumbnails.clear();
	techSpecFiles.clear();

	hasTechSpecDir = false;
	techSpecListId = -1;
}

Item* FtpData::getRootItem()
{
	return rootItem;
}

QList<QString>* FtpData::getParamList()
{
	return &paramList;
}

//void FtpData::loadDirectory(QString dir)
//{
//    ftpBaseDir = dir;
//    loadDirectory();
//}
void FtpData::loadDirectory(Item* item)
{
	ftpBaseDir = item->path;

	qDebug() << "Loading directory " << ftpBaseDir;

	if( ftp->state() == QFtp::Unconnected )
	{
		emit statusUpdated("Connecting to " + ftpHost);
		ftp->connectToHost(ftpHost, ftpPort);
		if (!ftpLogin.isEmpty())
			ftp->login(ftpLogin, ftpPassword);
		else
			ftp->login();
	}

	// FIXME: cd is done asynchronously!
	ftp->cd(ftpBaseDir);

	dbFilesQueued = 0;
	//rootItem = new Item();
	//rootItem->isDir = true;
	ftpCurrentItem = item;
	ftpCurrentDir = ftpBaseDir;

	//rootItem->path = ftpBaseDir;

	if(ftpListId != -1)
		qDebug() << "Ajaj";
	ftpListId = ftp->list();

	//qDebug() << "Job in progress";
}

void FtpData::ftpStateChanged(int state)
{
	switch( state )
	{
	case QFtp::HostLookup:
		emit statusUpdated(tr("Resolving %1...").arg(ftpHost));
		break;
	case QFtp::Connecting:
		emit statusUpdated(tr("Connecting..."));
		break;
	case QFtp::Connected:
		emit statusUpdated(tr("Connected."));
		qDebug() << "Connected. Browsing " << ftpBaseDir;
		break;
	case QFtp::LoggedIn:
		emit statusUpdated(tr("Logging in..."));
		break;
	case QFtp::Closing:
		emit statusUpdated(tr("Disconnecting..."));
		break;
	}
}

int FtpData::downloadPart(QString targetDir, Item *part)
{
//	emit statusUpdated("Downloading " + part->part);
//	QFile *f = new QFile(targetDir + "/" + part->part);
//	f->open(QFile::WriteOnly);
//	fileTasks[ftp->get(part->path + "/" + part->part, f)] = f;
//	return 0;
}

void FtpData::changeSettings(QString ftpHost, int ftpPort, bool ftpPassiveMode, QString ftpLogin, QString ftpPassword, QString ftpBaseDir)
{
	this->ftpHost = ftpHost;
	this->ftpPort = ftpPort;
	this->ftpPassiveMode = ftpPassiveMode;
	this->ftpLogin = ftpLogin;
	this->ftpPassword = ftpPassword;
	this->ftpBaseDir = ftpBaseDir;
}

//called for every item in the directory
void FtpData::ftpListInfo(const QUrlInfo &info)
{
	QString n = info.name();

	if( techSpecListId > -1 )
	{
		if( !info.isFile() )
			return;

		QString filePath = QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n;
		QFileInfo fi(filePath);
		QDir dir;

		if( !dir.exists(filePath) || info.lastModified() > fi.lastModified() )
		{
			QFile* f = new QFile(filePath);
			f->open(QFile::WriteOnly);

			//qDebug() << "Downloading techspec file" << QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n;
			techSpecFiles[ ftp->get(ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n, f) ] = f;
		}
		return;
	}

	if (info.isDir())
	{
		if( n == TECHSPEC_DIR )
		{
			//qDebug() << "Hey, I have found TECHSPEC directory... path is" << (ftpCurrentItem->path + n + "/") << "last edit is" << info.lastModified();
			hasTechSpecDir = true;
		} else {
			Item *i = new Item();
			i->isDir = true;
			i->name = n;
			i->parent = ftpCurrentItem;
			i->path = ftpCurrentItem->path + n + "/";
			i->server = rootItem->server;
			ftpCurrentItem->children.append(i);

			emit updateAvailable();
		}


		//qDebug() << "reading directory" << n;
		//dirsToList << p;
	}/*else if (info.isFile() && n.right(n.length() - n.lastIndexOf('.') - 1) == "db")
	{
		emit statusUpdated("Loading " + n);
		qDebug() << "Loading db file " << ftpCurrentDir << "/" << n;
		Item *p = new Item();
		p->path = ftpCurrentDir;
		p->parent = ftpCurrentItem;
		dbFilesQueued++;
		int id = ftp->get(ftpCurrentDir + "/" + n);
		partTasks[id] = p;
	}*/
	else if( info.isFile() ) {
		File *f = new File;
		//f->isDir = false;
		f->name = n;
		f->lastModified = info.lastModified();

		if( n.endsWith(".png", Qt::CaseInsensitive) || n.endsWith(".jpg", Qt::CaseInsensitive) )
		{
			thumbnails << f;
			return;
		}

		f->path = ftpCurrentItem->path + n;
		f->parentItem = ftpCurrentItem;
		//f->parent = ftpCurrentItem;
		//f->server = rootItem->server;

		ftpCurrentItem->files.append(f);
	}
}

//void FtpData::techSpecListInfo(const QUrlInfo &info)
//{
//	if( !info.isFile() )
//		return;

//	QString n = info.name();

//	QFile* f = new QFile(QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n);
//	f->open(QFile::WriteOnly);

//	techSpecFiles[ ftp->get(ftpCurrentItem->path + "/" + TECHSPEC_DIR + "/" + n) ] = f;
//}

void FtpData::ftpCommandFinished(int id, bool error)
{
	if (error)
	{
		//emit statusUpdated( tr("FTP error: %1").arg(ftp->errorString()) );
		emit errorOccured(ftp->errorString());
		qDebug() << "FTP error: " << ftp->errorString();
	} else if (id == ftpListId) { //finished listing a directory
		if (dirsToList.isEmpty())
		{
			ftpListId = -1;

			//qDebug() << "All done";
			emit allPartsDownloaded(ftpCurrentItem);

			if( hasTechSpecDir )
			{
				ftp->cd(ftpBaseDir + "/" + TECHSPEC_DIR);
				techSpecListId = ftp->list();

				hasTechSpecDir = false;

				QDir dir;
				dir.mkpath( QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + ftpCurrentItem->path + "/" + TECHSPEC_DIR );
			}

			// Check & download thumbnails
			foreach(File* file, ftpCurrentItem->files)
			{
				for(int i = 0; i < thumbnails.size(); i++)
				{
					//qDebug() << "file " << file->name.section('.', 0, 0) << "checking for thumbnail in" << thumbnails[i].section('.', -2, -2);
					if( file->name.section('.', 0, 0) == thumbnails[i]->name.section('.', -2, -2) )
					{
						QString thumbPath = QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + ftpCurrentItem->path + "/" + thumbnails[i]->name;
						QFileInfo fi(thumbPath);

						if( !QFile::exists(thumbPath) || file->lastModified > fi.lastModified() )
						{
							QDir dir;
							dir.mkpath(QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + ftpCurrentItem->path);
							QFile *f = new QFile(thumbPath);
							f->open(QFile::WriteOnly);
							file->openFtpFile = f;
							file->pixmapPath = thumbPath;

							qDebug() << "Downloading pixmap " << QString(ftpCurrentItem->path + "/" + thumbnails[i]->name) << "and saving to" << thumbPath;
							partPicTasks[ ftp->get(ftpCurrentItem->path + "/" + thumbnails[i]->name, f) ] = file;

							thumbnails.removeAt(i--);
						} else file->pixmap = QPixmap(thumbPath).scaledToWidth(100);
						break;
					}

				}
			}
			return;
		}

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
	} else if( techSpecFiles.contains(id) ) {
		QFile* f = techSpecFiles[id];
		f->close();

		techSpecFiles.remove(id);

		//qDebug() << "tech spec file" << f->fileName() << "downloaded";

		if( techSpecFiles.isEmpty() )
			sendTechSpecUrl(ftpCurrentItem);
	} else if (partPicTasks.contains(id)) { //finished downloading a picture
		File *file = partPicTasks[id];
		partPicTasks.remove(id);

		if (file->openFtpFile)
			file->openFtpFile->close();
		delete file->openFtpFile;
		file->openFtpFile = 0;

		if (!error)
		{
			file->pixmap = QPixmap(file->pixmapPath);
			emit gotPicture(file->parentItem);
		}
		qDebug() << "Pixmap" << file->pixmapPath << "downloaded.";
	}else if (partTasks.contains(id)) //finished downloading a .db file
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
			loadFile(p, settings);

			emit statusUpdated("Got " + p->name);
			qDebug() << "Got " << p->name;
			emit partDownloaded(p);
		}

		if (ftpListId == -1 && dbFilesQueued <= 0)
		{
			qDebug() << "All done";
			//emit allPartsDownloaded();
		}

	}else if (fileTasks.contains(id)) //actual file downloaded
	{
		qDebug() << "Finished downloading " << fileTasks[id]->name;
		emit statusUpdated("Finished downloading " + fileTasks[id]->name);

		QFile *f = fileTasks[id]->openFtpFile;
		fileTasks.remove(id);

		delete f;

		filesToDownload.removeFirst();

		if( filesToDownload.isEmpty() )
			emit filesDownloaded();
		else
			downloadFile(filesToDownload.first());
	}
}

void FtpData::loadFile(Item *p, QSettings &settings)
{
//	p->name = settings.value("name", QString()).toString();
//	p->pixmapFile = settings.value("img", QString()).toString();
//	p->part = settings.value("part", QString()).toString(); //TODO: more parts, more formats

//	//load params
//	settings.beginGroup("params");
//	foreach(QString s, settings.allKeys())
//	{
//		p->params[s] = settings.value(s);
//		if (!paramList.contains(s))
//			paramList.append(s);
//	}
//	settings.endGroup();

//	//TODO: some param processing functions (but probably only after everything is loaded)

//	p->parent->children.append(p);
//	p->id = p->parent->children.size() - 1;

//	//request picture download
//	if (!p->pixmapFile.isEmpty())
//	{
//		QFile *f = new QFile(QDir::homePath() + "/.zimaparts/pics/" + p->pixmapFile);
//		f->open(QFile::WriteOnly);
//		p->openFtpFile = f;
//		qDebug() << "Downloading pixmap " << QString(p->path + "/" + p->pixmapFile);
//		int id = (ftp->get(p->path + "/" + p->pixmapFile, f));
//		partPicTasks[id] = p;
//	}
}

void FtpData::downloadFiles(QList<File*> files, QString dir)
{
	targetDir = dir;
	filesToDownload = files;

	if( filesToDownload.size() > 0 )
		downloadFile(filesToDownload[0]);
}

void FtpData::downloadFile(File* file)
{
	emit statusUpdated("Downloading " + file->path);

	file->openFtpFile = new QFile(targetDir + "/" + file->name);
	file->openFtpFile->open(QFile::WriteOnly);
	fileTasks[ ftp->get(file->path, file->openFtpFile) ] = file;

	return;
}


void FtpData::sendTechSpecUrl(Item* item)
{
	int s = techSpecIndexes.size();

	for(int i = 0; i < s; i++)
	{
		QString indexPath = QDir::homePath() + "/.zimaparts/cache/" + ftpHost + "/" + item->path + "/" + TECHSPEC_DIR + "/" + techSpecIndexes[i];

		if( QFile::exists(indexPath) )
		{
			emit techSpecAvailable( QUrl::fromLocalFile(indexPath) );
			return;
		}
	}

	if( item == rootItem )
		emit techSpecAvailable( QUrl("about:blank") );
	else
		sendTechSpecUrl(item->parent);
}

void FtpData::abort()
{
	ftp->abort();
}
