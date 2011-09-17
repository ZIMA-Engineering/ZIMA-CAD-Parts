#include <QDebug>

#include "filemodel.h"
#include "item.h"

FileModel::FileModel(QObject *parent) :
	QAbstractItemModel(parent), rootItem(0), thumbSize(64)
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

	switch( index.column() )
	{
	case 1:
		switch( role )
		{
		case Qt::DecorationRole:
			return rootItem->files.at( index.row() )->pixmap.scaledToWidth(thumbSize);
		case Qt::SizeHintRole:
			if( !rootItem->files.at( index.row() )->pixmap.isNull() )
				return QSize(thumbSize, thumbSize);
		}
		break;
	case 0:
		switch( role )
		{
		case Qt::DisplayRole:
			//qDebug() << "returning" << rootItem->files.at( index.row() )->name;
			return rootItem->files.at( index.row() )->name;
		case Qt::CheckStateRole:
			return rootItem->files.at( index.row() )->isChecked ? Qt::Checked : Qt::Unchecked;
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
	//emit layoutAboutToBeChanged();
	rootItem = static_cast<Item*>(index.internalPointer());
	qDebug() << rootItem->files.size() << "files in this dir.";
	emit layoutChanged();
	//emit dataChanged(QModelIndex(), QModelIndex());
}

void FileModel::downloadFiles(Item* item, QString dir)
{
	QList<File*> filesToDownload;

	foreach(File* f, item->files)
	{
		if( f->isChecked )
			filesToDownload << f;
	}

	item->server->ftpData->downloadFiles(filesToDownload, dir);
}

Item* FileModel::getRootItem()
{
	return rootItem;
}

void FileModel::setThumbnailSize(int size)
{
	thumbSize = size;
}

void FileModel::uncheckAll()
{
	foreach(File* f, rootItem->files)
		f->isChecked = false;
	emit dataChanged( index(0, 0), index(rootItem->files.size(), 0) );
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
