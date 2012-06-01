/*
  ZIMA-Parts
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
#include "basedatasource.h"

BaseDataSource::BaseDataSource(QObject *parent) :
    QObject(parent)
{
	rootItem = 0;
	dataSource = UNDEFINED;

	techSpecIndexes << "index.html" << "index.htm";
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

void BaseDataSource::loadSettings(QSettings& settings)
{
	label = settings.value("Label").toString();
}

void BaseDataSource::saveSettings(QSettings& settings)
{
	settings.setValue("DataSourceType", internalName());
	settings.setValue("Label", label);
}
