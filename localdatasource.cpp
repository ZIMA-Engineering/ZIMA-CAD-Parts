#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDebug>

#include "localdatasource.h"

LocalCopier::LocalCopier(QList<File*> files, QString dir) :
	files(files),
	target(dir)
{

}

LocalCopier::LocalCopier(File* file, QString dir) :
	target(dir)
{
	files << file;
}

void LocalCopier::run()
{
	foreach(File *f, files)
	{
		emit aboutToCopy(f);

		QFile in(f->path);
		QFile out(f->targetPath);

		in.open(QIODevice::ReadOnly);
		out.open(QIODevice::WriteOnly);

		char block[8096];
		f->bytesDone = 0;

		while( !in.atEnd() )
		{
			qint64 bytes = in.read(block, sizeof(block));

			if (bytes <= 0)
				break;

			f->bytesDone += bytes;

			out.write(block, bytes);

			emit progress(f);
		}

		emit fileCopied(f);

		files.removeOne(f);
	}

	emit done();
}

void LocalCopier::addFiles(QList<File*> files)
{
	this->files << files;
}

LocalDataSource::LocalDataSource(QObject *parent) :
	BaseDataSource(parent),
	copier(0)
{
	dataSource = LOCAL;

	label = tr("New local folder");
}

QString LocalDataSource::internalName()
{
	return "local";
}

void LocalDataSource::loadDirectory(Item* item)
{
	QDir dir(item->path);
	QFileInfoList entries = dir.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot );
	QStringList thumbnails;

	int cnt = entries.count();

	for(int i = 0; i < cnt; i++)
	{
		if( entries[i].isDir() )
		{
			if( entries[i].fileName() == TECHSPEC_DIR )
			{
				sendTechSpecUrl(item);
				continue;
			}

			Item *it = new Item;
			it->isDir = true;
			it->name = entries[i].fileName();
			it->parent = item;
			it->path = item->path + it->name + "/";
			it->server = item->server;

			item->children.append(it);
		} else {
			if( entries[i].fileName().endsWith(".png", Qt::CaseInsensitive) || entries[i].fileName().endsWith(".jpg", Qt::CaseInsensitive) )
			{
				thumbnails << entries[i].fileName();
				continue;
			}

			File *f = new File;
			f->name = entries[i].fileName();
			f->size = entries[i].size();
			f->path = item->path + f->name;
			f->parentItem = item;
			item->files.append(f);
		}
	}

	// Check for thumbnails
	foreach(File *f, item->files)
	{
		for(int i = 0; i < thumbnails.count(); i++)
		{
			if( f->name.section('.', 0, 0) == thumbnails[i].section('.', -2, -2) )
			{
				qDebug() << "Found pixmap for" << f->name << item->path + thumbnails[i];
				f->pixmapPath = item->path + thumbnails[i];
				f->pixmap = QPixmap(f->pixmapPath).scaledToWidth(100);
			}
		}
	}

	emit allPartsDownloaded(item);
}

void LocalDataSource::sendTechSpecUrl(Item* item)
{
	int s = techSpecIndexes.size();

	for(int i = 0; i < s; i++)
	{
		QString indexPath = item->path + TECHSPEC_DIR + "/" + techSpecIndexes[i];

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

void LocalDataSource::downloadFiles(QList<File*> files, QString dir)
{
	foreach(File *f, files)
		if( f->targetPath.isEmpty() )
			f->targetPath = dir + "/" + f->name;

	if( !copier )
	{
		copier = new LocalCopier(files, dir);

		connect(copier, SIGNAL(aboutToCopy(File*)), this, SLOT(aboutToCopy(File*)));
		connect(copier, SIGNAL(progress(File*)), this, SIGNAL(fileProgress(File*)));
		connect(copier, SIGNAL(fileCopied(File*)), this, SIGNAL(fileDownloaded(File*)));
		connect(copier, SIGNAL(done()), this, SIGNAL(filesDownloaded()));
	} else
		copier->addFiles(files);

	if( !copier->isRunning() )
		copier->start();
}

void LocalDataSource::downloadFile(File* file)
{

}

void LocalDataSource::resumeDownload()
{
	if( copier )
		copier->start();
}

void LocalDataSource::abort()
{
	if( copier )
		copier->terminate();
}

void LocalDataSource::loadSettings(QSettings& settings)
{
	BaseDataSource::loadSettings(settings);

	localPath = settings.value("Path", "").toString();
}

void LocalDataSource::saveSettings(QSettings& settings)
{
	BaseDataSource::saveSettings(settings);

	settings.setValue("Path", localPath);
}

void LocalDataSource::aboutToCopy(File *file)
{
	emit statusUpdated(tr("Copying ") + file->name);
}
