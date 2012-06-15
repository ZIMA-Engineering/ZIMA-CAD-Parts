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

#include <QDebug>
#include <QRegExp>

#include "filemodel.h"
#include "item.h"
#include "basedatasource.h"

FileModel::FileModel(QObject *parent) :
	QAbstractItemModel(parent), rootItem(0), thumbWidth(256)
{
}

int FileModel::columnCount(const QModelIndex &parent) const
{
	//qDebug() << "col count = " << 2 + colLabels.count();
	return 2 + colLabels.count();
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
	const int col = index.column();

	switch(col)
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
			else return QSize(thumbWidth, 0);
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

	if(col > 1 && role == Qt::DisplayRole && col-2 < colLabels.count())
	{
		//qDebug() << "Probing section" << rootItem->files.at( row )->name.section('.', 0, 0);
		//return rootItem->metadata->value(QString("%1/%2").arg(rootItem->files.at( row )->name.section('.', 0, 0)).arg(col-1), QString()).toString();
		return rootItem->metadata->getPartParam(rootItem->files.at(row)->name, col-1);
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

	if(section > 1 && section-2 < colLabels.count())
	{
		return colLabels[section - 2];
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
	{
		disconnect(rootItem->server, SIGNAL(thumbnailLoaded(File*)), this, SLOT(thumbnailDownloaded(File*)));
		disconnect(rootItem->server, SIGNAL(metadataReady(Item*)), this, SLOT(initMetadata(Item*)));
		disconnect(rootItem->server, SIGNAL(itemLoaded(Item*)), this, SLOT(itemLoaded(Item*)));

		if(rootItem->metadata)
			disconnect(rootItem->metadata, SIGNAL(retranslated()), this, SLOT(metadataRetranslated()));
	}

	if( !index.isValid() )
	{
		rootItem = 0;
		reset();
		return;
	}

	//emit layoutAboutToBeChanged();
	rootItem = static_cast<Item*>(index.internalPointer());
	connect(rootItem->server, SIGNAL(thumbnailLoaded(File*)), this, SLOT(thumbnailDownloaded(File*)));
	connect(rootItem->server, SIGNAL(metadataReady(Item*)), this, SLOT(initMetadata(Item*)));
	connect(rootItem->server, SIGNAL(itemLoaded(Item*)), this, SLOT(itemLoaded(Item*)));

	if(rootItem->metadata)
		initMetadata(rootItem);
	else if( colLabels.count() )
	{
		beginRemoveColumns(QModelIndex(), 2, 2 + colLabels.count());
		endRemoveColumns();
		colLabels.clear();
	}

	reset();
	emit layoutChanged();
}

void FileModel::initMetadata(Item *i)
{
	if(i != rootItem)
		return;

	connect(rootItem->metadata, SIGNAL(retranslated()), this, SLOT(metadataRetranslated()));

	colLabels = rootItem->metadata->getColumnLabels();

	//qDebug() << colLabels;

	beginInsertColumns(QModelIndex(), 2, 2 + colLabels.count());
	endInsertColumns();
	//emit headerDataChanged(Qt::Horizontal, 2, 2 + colLabels.count());

	emit requestColumnResize();
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
	file->scaledThumb = QPixmap();

	QModelIndex mi = index( file->parentItem->files.indexOf(file), 1 );

	emit dataChanged(mi, mi);
}

void FileModel::itemLoaded(Item *item)
{
	if(item == rootItem)
	{
		reset();
		emit requestColumnResize();
	}
}

void FileModel::metadataRetranslated()
{
	colLabels = rootItem->metadata->getColumnLabels();

	emit headerDataChanged(Qt::Horizontal, 2, colLabels.count()-1);
	emit requestColumnResize();
}
