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

#include <QStyle>
#include <QDir>
#include <QDebug>
#include "basedatasource.h"
#include "mainwindow.h"

BaseDataSource::BaseDataSource(QObject *parent) :
    QObject(parent)
{
	rootItem = 0;
	dataSource = UNDEFINED;
}

Item* BaseDataSource::getRootItem()
{
	return rootItem;
}

QIcon BaseDataSource::itemIcon(Item *item)
{
	if( item == rootItem )
		return qApp->style()->standardIcon( QStyle::SP_DriveHDIcon );
	return qApp->style()->standardIcon( QStyle::SP_DirIcon );
}

QIcon BaseDataSource::dataSourceIcon()
{
	return qApp->style()->standardIcon( QStyle::SP_DriveHDIcon );
}

void BaseDataSource::deleteDownloadQueue()
{

}

void BaseDataSource::assignThumbnailsToFiles(Item *item, QList<Thumbnail*> thumbnails)
{
	if(thumbnails.isEmpty())
		thumbnails << item->thumbnails();

	if(item->files.isEmpty() || thumbnails.isEmpty())
		return;

	foreach(File *f, item->files)
	{
		f->thumbnail = 0;
		f->thumbnails.clear();

		QString fileNamePrefix = f->name.section('.', 0, 0);

		foreach(Thumbnail *thumb, thumbnails)
		{
			if(fileNamePrefix == thumb->name())
			{
				f->thumbnails << thumb;

				if(!f->thumbnail || thumb->language() == currentMetadataLang)
					f->thumbnail = thumb;
			}
		}

		if(f->thumbnail && f->thumbnail->isReady())
			emit thumbnailLoaded(f);
	}
}

void BaseDataSource::determineFileVersions(Item *item)
{
	foreach(File *f, item->files)
	{
		File *newest = f;

		foreach(File *g, item->files)
		{
			if(f->baseName() != g->baseName())
				continue;

			if(g->version > newest->version)
				newest = g;
		}

		newest->newestVersion = true;
	}
}

void BaseDataSource::sendTechSpecUrl(Item* item)
{
	QStringList filters;
	filters << "index_??.html" << "index_??.htm" << "index.html" << "index.htm";

	QDir dir(getTechSpecPathForItem(item));
	QStringList indexes = dir.entryList(filters, QDir::Files | QDir::Readable);

	if(indexes.isEmpty())
	{
		if( item == rootItem )
			emit techSpecAvailable(QUrl("about:blank"));
		else
			sendTechSpecUrl(item->parent);

		return;
	}

	QString selectedIndex = indexes.first();
	indexes.removeFirst();

	foreach(QString index, indexes)
	{
		QString prefix = index.section('.', 0, 0);

		if(prefix.lastIndexOf('_') == prefix.count()-3 && prefix.right(2) == currentMetadataLang)
			selectedIndex = index;
	}

	emit techSpecAvailable(QUrl::fromLocalFile(dir.path() + "/" + selectedIndex));
}

void BaseDataSource::retranslate(QString lang)
{
	if(lang.isEmpty())
		currentMetadataLang = MainWindow::getCurrentMetadataLanguageCode().left(2);
	else
		currentMetadataLang = lang;
}

void BaseDataSource::loadSettings(QSettings& settings)
{
	label = settings.value("Label").toString();
}

void BaseDataSource::saveSettings(QSettings& settings)
{
	settings.setValue("DataSourceType", internalName());
	settings.setValue("Label", label);
}

void BaseDataSource::createMetadata(Item *item)
{
	item->metadata = new Metadata(item);

	connect(item->metadata, SIGNAL(ready(Item*)), this, SIGNAL(metadataReady(Item*)));
	connect(item->metadata, SIGNAL(includeRequired(Item*,QString)), this, SIGNAL(metadataInclude(Item*,QString)));
	connect(item->metadata, SIGNAL(includeRequireCancelled(Item*)), this, SIGNAL(metadataIncludeCancelled(Item*)));

	item->metadata->init();
}

void BaseDataSource::assignTechSpecUrlToItem(QString url, Item *item, QString lang, bool overwrite)
{

}

void BaseDataSource::assignPartsIndexUrlToItem(QString url, Item *item, QString lang, bool overwrite)
{

}
