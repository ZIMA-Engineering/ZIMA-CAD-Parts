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

#include <QDebug>

#include "filemodel.h"
#include "item.h"
#include "basedatasource.h"

FileModel::FileModel(QObject *parent) :
	QAbstractItemModel(parent), rootItem(0), thumbWidth(256)
{
}

int FileModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

int FileModel::rowCount(const QModelIndex &parent) const
{
	if( rootItem == 0 )
		return 0;

	//qDebug() << "Asking about rowCount" << rootItem->files.size();

	if( !parent.isValid() )
		return rootItem->files.size();
	return 0;
}

QModelIndex FileModel::parent(const QModelIndex &index) const
{
	//qDebug() << "Asking about parent";
	return QModelIndex();
}

QModelIndex FileModel::index(int row, int column, const QModelIndex &parent) const
{
	if( rootItem == 0 )
		return QModelIndex();

	/*if( !parent.isValid() )
		return QModelIndex();*/

	//qDebug() << "Asking about index" << row << column;

	return createIndex(row, column);
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
	if( rootItem == 0 || rootItem->files.isEmpty() )
		return QVariant();

	//qDebug() << "Asking about data" << rootItem->files.at( index.row() )->name << role;
	//qDebug() << "Column" << index.column();

	const int row = index.row();

	switch( index.column() )
	{
	case 1:
		switch( role )
		{
		case Qt::DecorationRole:
			if( !rootItem->files.at( row )->pixmap.isNull() )
			{
				if( rootItem->files.at( row )->scaledThumb.isNull() )
					rootItem->files.at( row )->scaledThumb = rootItem->files.at( row )->pixmap.scaledToWidth( thumbWidth );
				return rootItem->files.at( row )->scaledThumb;
			}
		case Qt::SizeHintRole:
			if( !rootItem->files.at( row )->pixmap.isNull() )
                                return QSize(thumbWidth, thumbWidth);
		case Qt::ToolTipRole:
			if( !rootItem->files.at( row )->pixmap.isNull() )
				return QString("<img src=\"%1\" width=\"%2\">").arg( rootItem->files.at( row )->pixmapPath ).arg( previewWidth );
		}
		break;
	case 0:
		switch( role )
		{
		case Qt::DisplayRole:
			//qDebug() << "returning" << rootItem->files.at( index.row() )->name;
			return rootItem->files.at( row )->name;
		case Qt::CheckStateRole:
			return rootItem->files.at( row )->isChecked ? Qt::Checked : Qt::Unchecked;
		case Qt::DecorationRole: {
			QPixmap tmp = rootItem->files.at( row )->icon();

			if( !tmp.isNull() )
				return tmp.scaledToWidth(16);
		}
		}
		break;
	}

	return QVariant();
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if( value.toString().isEmpty() )
		return false;

	if( role == Qt::CheckStateRole )
		rootItem->files[ index.row() ]->isChecked = value.toBool();

	emit dataChanged(index, index);

	return true;
}

QVariant FileModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if( role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch( section )
	{
	case 1:
		return tr("Thumbnail");
	case 0:
		return tr("Part name");
	}

        return QVariant();
}

bool FileModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	return false;
}

Qt::ItemFlags FileModel::flags(const QModelIndex &index) const
{
	if( index.column() == 0 )
		return Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
	else return QAbstractItemModel::flags(index);
}

void FileModel::setRootIndex(const QModelIndex &index)
{
	if( rootItem )
		disconnect(rootItem->server, SIGNAL(gotThumbnail(File*)), this, SLOT(thumbnailDownloaded(File*)));

	if( !index.isValid() )
	{
		rootItem = 0;
		reset();
		return;
	}

	//emit layoutAboutToBeChanged();
	rootItem = static_cast<Item*>(index.internalPointer());
	connect(rootItem->server, SIGNAL(gotThumbnail(File*)), this, SLOT(thumbnailDownloaded(File*)));

	reset();
	emit layoutChanged();
}

Item* FileModel::getRootItem()
{
	return rootItem;
}

void FileModel::setThumbWidth(int size)
{
	thumbWidth = size;
}

void FileModel::setPreviewWidth(int size)
{
	previewWidth = size;
}

void FileModel::thumbnailDownloaded(File *file)
{
	QModelIndex mi = index( file->parentItem->files.indexOf(file), 1 );

	emit dataChanged(mi, mi);
}
