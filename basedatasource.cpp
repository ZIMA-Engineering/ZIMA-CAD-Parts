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
