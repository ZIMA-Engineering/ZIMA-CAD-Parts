#include "baseremotedatasource.h"

#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QStyle>
#include <QDebug>

QIcon BaseRemoteDataSource::itemIcon(Item *item)
{
	if( item == rootItem )
		return qApp->style()->standardIcon( QStyle::SP_DriveNetIcon );
	return qApp->style()->standardIcon( QStyle::SP_DirIcon );
}

QIcon BaseRemoteDataSource::dataSourceIcon()
{
	return qApp->style()->standardIcon( QStyle::SP_DriveNetIcon );
}

void BaseRemoteDataSource::sendTechSpecUrl(Item* item)
{
	int s = techSpecIndexes.size();

	for(int i = 0; i < s; i++)
	{
		QString indexPath = cacheDirPath() + "/" + remoteHost + "/" + item->path + "/" + TECHSPEC_DIR + "/" + techSpecIndexes[i];

		if( QFile::exists(indexPath) )
		{
			emit techSpecAvailable( QUrl::fromLocalFile(indexPath) );
			return;
		}
	}

	if( item == rootItem )
		emit techSpecAvailable( QUrl("about:blank") );
	else
		sendTechSpecUrl(item->parent);
}

void BaseRemoteDataSource::loadSettings(QSettings& settings)
{
	BaseDataSource::loadSettings(settings);

	remoteHost = settings.value("Host", "localhost").toString();
	remotePort = settings.value("Port", "21").toInt();
	remoteBaseDir = settings.value("BaseDir", "/").toString();
	remoteLogin = settings.value("Login", "").toString();
	remotePassword = settings.value("Password", "").toString();
}

void BaseRemoteDataSource::saveSettings(QSettings& settings)
{
	BaseDataSource::saveSettings(settings);

	settings.setValue("Host", remoteHost);
	settings.setValue("Port", remotePort);
	settings.setValue("BaseDir", remoteBaseDir);
	settings.setValue("Login", remoteLogin);
	settings.setValue("Password", remotePassword);
}

QString BaseRemoteDataSource::cacheDirPath()
{
	return QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
}
