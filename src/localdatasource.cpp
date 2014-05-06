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

#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDebug>

#include "localdatasource.h"
#include "metadata.h"
#include "mainwindow.h"

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

void LocalDataSource::loadRootItem(Item *item)
{
	loadItemLogo(item);
}

void LocalDataSource::loadItemLogo(Item *item)
{
	QString logoPath = item->path + "/" + TECHSPEC_DIR + "/";

	if(QFile::exists(logoPath + LOGO_FILE)) {
		item->logo = QPixmap(logoPath + LOGO_FILE);
		item->showText = false;
	} else if(QFile::exists(logoPath + LOGO_TEXT_FILE)) {
		item->logo = QPixmap(logoPath + LOGO_TEXT_FILE);
		item->showText = true;
	}
}

QString LocalDataSource::getTechSpecPathForItem(Item *item)
{
	return getPathForItem(item) + TECHSPEC_DIR;
}

QString LocalDataSource::getPathForItem(Item *item)
{
	return item->path;
}

QString LocalDataSource::getRelativePathForItem(Item *item)
{
	QString path = item->path;
	path.remove(0, localPath.length());
	return path;
}

QString LocalDataSource::name()
{
	return label;
}

void LocalDataSource::loadDirectory(Item* item)
{
	if(!item->children.isEmpty() || !item->files.isEmpty())
	{
		emit itemLoaded(item);
		return;
	}

	QDir dir(item->path);
	QFileInfoList entries = dir.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot );

	int cnt = entries.count();

	for(int i = 0; i < cnt; i++)
	{
		if( entries[i].isDir() )
		{
			if( entries[i].fileName() == TECHSPEC_DIR )
			{
				sendTechSpecUrl(item);

				if(item == rootItem && QFile::exists(item->path + "/" + TECHSPEC_DIR + "/" + METADATA_FILE))
					createMetadata(item);

				continue;
			}

			Item *it = new Item;
			it->isDir = true;
			it->name = entries[i].fileName();
			it->parent = item;
			it->path = item->path + it->name + "/";
			it->server = item->server;

			loadItemLogo(it);

			item->children.append(it);

			if(QFile::exists(it->path + TECHSPEC_DIR + "/" + METADATA_FILE))
				createMetadata(it);

		} else {
			if( entries[i].fileName().endsWith(".png", Qt::CaseInsensitive) || entries[i].fileName().endsWith(".jpg", Qt::CaseInsensitive) )
			{
				item->addThumbnail(new Thumbnail(item, entries[i].fileName()));
				continue;
			}

			File *f = new File;
			f->setName(entries[i].fileName());
			f->size = entries[i].size();
			f->path = item->path + f->name;
			f->parentItem = item;
			item->files.append(f);
		}
	}

	// Check for thumbnails
//	if(!item->files.isEmpty() && !thumbnails.isEmpty())
//	{
//		foreach(File *f, item->files)
//		{
//			f->pixmapPath = "";

//			QString fileNamePrefix = f->name.section('.', 0, 0);

//			for(int i = 0; i < thumbnails.count(); i++)
//			{
//				QString thumbPrefix = thumbnails[i].section('.', -2, -2);
//				QString thumbName;
//				bool isLocalized = false;

//				if(thumbPrefix.lastIndexOf('_') == thumbPrefix.count()-3)
//				{
//					thumbName = thumbPrefix.left(thumbPrefix.count()-3);
//					isLocalized = true;
//				} else thumbName = thumbPrefix;

//				if(fileNamePrefix == thumbName)
//				{
//					// When there's no thumbnail set yet, pick first available
//					if(f->pixmapPath.isEmpty())
//						f->pixmapPath = item->path + thumbnails[i];

//					// Localized thumbnail has precedence/
//					if(isLocalized && thumbPrefix.right(2) == currentMetadataLang)
//					{
//						f->pixmapPath = item->path + thumbnails[i];
//						break;
//					}
//				}
//			}

//			if(!f->pixmapPath.isEmpty())
//				f->pixmap = QPixmap(f->pixmapPath).scaledToWidth(100);
//		}
//	}

	assignThumbnailsToFiles(item);
	determineFileVersions(item);

	emit itemLoaded(item);
}

void LocalDataSource::deleteFiles(QList<File*> files)
{
	foreach(File *f, files)
	{
		if(QFile::remove(f->path))
		{
			f->parentItem->metadata->deletePart(f->name);
			f->parentItem->files.removeOne(f);

			foreach(Thumbnail *thumb, f->thumbnails)
			{
				qDebug() << "Remove thumbnail" << thumb->absolutePath();
				QFile::remove(thumb->absolutePath());
			}

			// FIXME: delete f->thumbnails?
			delete f;

		} else
			emit fileError(BaseDataSource::Delete, new BaseDataSource::Error(f, tr("Unable to delete")));
	}

	emit filesDeleted();
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
	Q_UNUSED(file);
}

void LocalDataSource::resumeDownload()
{
	copier->start();
}

void LocalDataSource::abort()
{
	copier->terminate();
}

void LocalDataSource::assignTechSpecUrlToItem(QString url, Item *item, QString lang, bool overwrite)
{
	QByteArray htmlIndex = QString("<html>\n"
	                               "	<head>\n"
	                               "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
	                               "	</head>\n"
	                               "</html>\n").arg(url).toUtf8();
	QDir techSpecDir(item->path + "/" + TECHSPEC_DIR);

	if(!techSpecDir.exists())
		techSpecDir.mkdir(techSpecDir.absolutePath());

	QFile indexFile(techSpecDir.absoluteFilePath("index_" + lang + ".html"));

	if(indexFile.exists() && !overwrite)
	{
		emit techSpecsIndexAlreadyExists(item);
		return;
	}

	if(!indexFile.open(QIODevice::WriteOnly))
		return; // FIXME: Notify user on failure?

	indexFile.write(htmlIndex);
	indexFile.close();
}

void LocalDataSource::assignPartsIndexUrlToItem(QString url, Item *item, QString lang, bool overwrite)
{
	QByteArray htmlIndex = QString("<html>\n"
	                               "	<head>\n"
	                               "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
	                               "	</head>\n"
	                               "</html>\n").arg(url).toUtf8();
	QDir techSpecDir(item->path + "/" + TECHSPEC_DIR);

	if(!techSpecDir.exists())
		techSpecDir.mkdir(techSpecDir.absolutePath());

	QFile indexFile(techSpecDir.absoluteFilePath("index-parts_" + lang + ".html"));

	if(indexFile.exists() && !overwrite)
	{
		emit partsIndexAlreadyExists(item);
		return;
	}

	if(!indexFile.open(QIODevice::WriteOnly))
		return; // FIXME: Notify user on failure?

	indexFile.write(htmlIndex);
	indexFile.close();
}

void LocalDataSource::aboutToCopy(File *file)
{
	emit statusUpdated(tr("Copying ") + file->name);
}

QString LocalDataSource::pathToDataRoot()
{
	return localPath;
}
