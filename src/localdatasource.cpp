/*
  ZIMA-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011 Jakub Skokan <aither@havefun.cz>

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

#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDebug>

#include "localdatasource.h"

LocalCopier::LocalCopier(QList<File*> files) :
	files(files)
{

}

LocalCopier::LocalCopier(File* file)
{
	files << file;
}

void LocalCopier::run()
{
	if( !files.count() )
		return;

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

void LocalCopier::addFile(File* file)
{
	files << file;
}

void LocalCopier::addFiles(QList<File*> files)
{
	this->files << files;
}

LocalDataSource::LocalDataSource(QObject *parent) :
	BaseDataSource(parent)
{
	dataSource = LOCAL;

	copier = new LocalCopier();

	connect(copier, SIGNAL(aboutToCopy(File*)), this, SLOT(aboutToCopy(File*)));
	connect(copier, SIGNAL(progress(File*)), this, SIGNAL(fileProgress(File*)));
	connect(copier, SIGNAL(fileCopied(File*)), this, SIGNAL(fileDownloaded(File*)));
	connect(copier, SIGNAL(done()), this, SIGNAL(filesDownloaded()));
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

void LocalDataSource::addFileToDownload(File *f)
{
	copier->addFile(f);
}

void LocalDataSource::downloadFiles(QList<File*> files, QString dir)
{
	foreach(File *f, files)
		if( f->targetPath.isEmpty() )
			f->targetPath = dir + "/" + f->name;

	copier->addFiles(files);

	if( !copier->isRunning() )
		copier->start();
}

void LocalDataSource::downloadFile(File* file)
{

}

void LocalDataSource::resumeDownload()
{
	copier->start();
}

void LocalDataSource::abort()
{
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
