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

#include "downloadmodel.h"
#include <QDebug>
#include <QApplication>
#include <QStyleOptionProgressBarV2>
#include "basedatasource.h"
#include "datatransfer.h"
#include "transferhandler.h"
#include "item.h"

void DownloadDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if( index.column() == 2 )
	{
		File* f = static_cast<File*>( index.internalPointer() );
		QStyleOptionProgressBarV2 opts;
//		const int row = index.row();

		opts.text = QString("%1 %").arg( f->size ? (f->bytesDone * 100 / f->size) : 0 );
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

DownloadModel::DownloadModel(TransferHandler *handler, QObject *parent) :
	QAbstractItemModel(parent),
	m_downloading(false),
	m_handler(handler)
{
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
	if( !parent.isValid() )
		return queue.count();

	return 0;
}

QModelIndex DownloadModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();
}

QModelIndex DownloadModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	if( queue.isEmpty() )
		return QModelIndex();

	if( column == 2 )
		return createIndex(row, column, queue[row]);

	return createIndex(row, column);
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
	if( queue.isEmpty() )
		return QVariant();

	const int row = index.row();

	switch( role )
	{
	case Qt::DisplayRole:
		switch( index.column() )
		{
		case 0:
			return queue[row]->name;
		case 1:
			return queue[row]->targetPath;
		case 2:
			return queue[row]->bytesDone;
		default:
			break;
		}
	default:
		break;
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

	return "unknown data";
}

QList<File*> DownloadModel::files()
{
	QList<File*> tmp;

	foreach(File *f, queue)
	{
		tmp << f;
	}

	return tmp;
}

bool DownloadModel::isEmpty() const
{
	return queue.isEmpty();
}

bool DownloadModel::isDownloading() const
{
	return m_downloading;
}

void DownloadModel::enqueue(File *f)
{
	int cnt = queue.count();

	beginInsertRows(QModelIndex(), cnt, cnt);

	queue << f;

	endInsertRows();

	if(!f->transfer)
		return;

	connect(f->transfer, SIGNAL(progress(File*)), this, SLOT(fileChanged(File*)));
	connect(f->transfer, SIGNAL(done(File*)), this, SLOT(fileDownloaded(File*)));

	m_downloading = true;
}

void DownloadModel::enqueue(QList<File *> list)
{
	int cnt = queue.count();

	beginInsertRows(QModelIndex(), cnt, cnt + list.count() - 1);

	queue << list;

	endInsertRows();

	m_downloading = true;
}

void DownloadModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, queue.count() - 1);
	stop();
	m_handler->clearQueue();
	queue.clear();
	endRemoveRows();
}

void DownloadModel::fileChanged(File *file)
{
	QModelIndex i = index(queue.indexOf(file), 2);

	emit dataChanged(i, i);
}

void DownloadModel::fileDownloaded(File *file)
{
	int i = queue.indexOf(file);

	if (i != -1)
	{
		beginRemoveRows(QModelIndex(), i, i);

		File *f = queue.takeAt(i);

		endRemoveRows();

		if(f->transfer)
			delete f->transfer;
	}
}

void DownloadModel::stop()
{
	m_downloading = false;
	m_handler->stopDownload();
}

void DownloadModel::resume()
{
	m_downloading = true;
	m_handler->resumeDownload();
}
