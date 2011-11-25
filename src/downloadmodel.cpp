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

#include "downloadmodel.h"
#include <QDebug>
#include <QApplication>
#include <QStyleOptionProgressBarV2>
#include "basedatasource.h"

void DownloadDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if( index.column() == 2 )
	{
		File* f = static_cast<File*>( index.internalPointer() );
		QStyleOptionProgressBarV2 opts;
		const int row = index.row();

		opts.text = QString("%1 %").arg( f->bytesDone * 100 / f->size );
		opts.maximum = f->size;
		opts.minimum = 0;
		opts.progress = f->bytesDone;
		opts.rect = option.rect;
		opts.textVisible = true;
		opts.state = QStyle::State_Enabled;

		QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opts, painter);
	}
	else
		QItemDelegate::paint(painter, option, index);
}

DownloadModel::DownloadModel(QObject *parent) :
	QAbstractItemModel(parent),
	queue(0)
{
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
	if( !parent.isValid() )
		return queue ? queue->count() : 0;
	return 0;
}

QModelIndex DownloadModel::parent(const QModelIndex &index) const
{
	return QModelIndex();
}

QModelIndex DownloadModel::index(int row, int column, const QModelIndex &parent) const
{
	if( !queue )
		return QModelIndex();

	if( column == 2 )
		return createIndex(row, column, (*queue)[row]);
	return createIndex(row, column);
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
	if( !queue )
		return QVariant();

	const int row = index.row();

	switch( role )
	{
	case Qt::DisplayRole:
		switch( index.column() )
		{
		case 0:
			return (*queue)[row]->name;
		case 1:
			return (*queue)[row]->targetPath;
		case 2:
			return (*queue)[row]->bytesDone;
		default:break;
		}
	default:break;
	}
	return QVariant();
}

QVariant DownloadModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if( role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch(section)
	{
	case 0:
		return tr("Part");
	case 1:
		return tr("Destination");
	case 2:
		return tr("Progress");
	}
}

void DownloadModel::setQueue(QList<File*>* q)
{
	queue = q;
	reset();
}

void DownloadModel::fileChanged(File *file)
{
	QModelIndex i = index(queue->indexOf(file), 2);

	emit dataChanged(i, i);
}

void DownloadModel::fileDownloaded(File *file)
{
	queue->removeOne(file);

	emit layoutChanged();
}

void DownloadModel::queueChanged()
{
	emit layoutChanged();
}
