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
				return QSize(64, 64);
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

	qDebug() << rootItem->files.size() << "files in this dir.";

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

//QModelIndex FileModel::mapToSource(const QModelIndex &proxyIndex) const
//{

//}

//QModelIndex FileModel::mapFromSource(const QModelIndex &sourceIndex) const
//{
//	if( !sourceIndex.isValid() )
//		return QModelIndex();

//	return index();
//}

//bool FileModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
//{
//	if( !source_parent.isValid() )
//	{
//		qDebug() << "Asking with invalid index";
//		return true;
//	}

//	Item *i = static_cast<Item*>(source_parent.internalPointer());

//	qDebug() << "Asking about " << i->name;

//	if( !i->isDir && !i->isServer )
//		return true;

//	//	int childCount = source_parent.model()->rowCount(source_parent);

//	//	if( childCount == 0 )
//	//		return false;

//	//	for (int i = 0; i < childCount; ++i)
//	//	{
//	//		if( filterAcceptsRow(i, source_parent) )
//	//			return true;
//	//	}

//	//	return false;

//	//-----------------
////	if (filterAcceptsRowItself(source_row, source_parent))
////		return true;

////	//accept if any of the parents is accepted on it's own merits
////	QModelIndex parent = source_parent;
////	while (parent.isValid()) {
////		if (filterAcceptsRowItself(parent.row(), parent.parent()))
////			return true;
////		parent = parent.parent();
////	}

////	//accept if any of the children is accepted on it's own merits
////	if (hasAcceptedChildren(source_row, source_parent)) {
////		return true;
////	}

////	return false;
//}

//bool FileModel::filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const
//{
//	//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
//}

//bool FileModel::hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const
//{
//	QModelIndex item = sourceModel()->index(source_row,0,source_parent);
//	if (!item.isValid()) {
//		//qDebug() << "item invalid" << source_parent << source_row;
//		return false;
//	}

//	//check if there are children
//	int childCount = item.model()->rowCount(item);
//	if (childCount == 0)
//		return false;

//	for (int i = 0; i < childCount; ++i) {
//		if (filterAcceptsRowItself(i, item))
//			return true;
//		//recursive call
//		if (hasAcceptedChildren(i, item))
//			return true;
//	}

//	return false;
//}
