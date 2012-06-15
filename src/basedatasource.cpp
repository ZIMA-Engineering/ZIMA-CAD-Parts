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

void BaseDataSource::assignThumbnailsToFiles(Item *item, QStringList thumbnails)
{
	QString itemPath = getPathForItem(item);

	if(thumbnails.isEmpty())
	{
		QStringList filters;
		filters << "*.jpg" << "*.png";

		QDir dir(itemPath);
		thumbnails = dir.entryList(filters, QDir::Files | QDir::Readable);
	}

	if(!item->files.isEmpty() && !thumbnails.isEmpty())
	{
		foreach(File *f, item->files)
		{
			f->pixmapPath = "";

			QString fileNamePrefix = f->name.section('.', 0, 0);

			foreach(QString thumb, thumbnails)
			{
				QString thumbPrefix = thumb.section('.', -2, -2);
				QString thumbName;
				bool isLocalized = false;

				if(thumbPrefix.lastIndexOf('_') == thumbPrefix.count()-3)
				{
					thumbName = thumbPrefix.left(thumbPrefix.count()-3);
					isLocalized = true;
				} else thumbName = thumbPrefix;

				if(fileNamePrefix == thumbName)
				{
					// When there's no thumbnail set yet, pick first available
					if(f->pixmapPath.isEmpty())
						f->pixmapPath = itemPath + "/" + thumb;

					// Localized thumbnail has precedence
					if(isLocalized && thumbPrefix.right(2) == currentMetadataLang)
					{
						f->pixmapPath = itemPath + "/" + thumb;
						break;
					}
				}
			}

			if(!f->pixmapPath.isEmpty())
			{
				f->pixmap = QPixmap(f->pixmapPath);

				if(!f->pixmap.isNull())
					emit thumbnailLoaded(f);
			}
		}
	}
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
