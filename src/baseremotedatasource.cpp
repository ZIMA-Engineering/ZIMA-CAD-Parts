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
	techSpecItem = item;
	techSpecUrlSent = false;

	checkAndSendTechSpecUrl(techSpecItem);
}

void BaseRemoteDataSource::checkAndSendTechSpecUrl(Item *item)
{
	if(techSpecUrlSent && !techSpecFilesUpdated)
		return;

	int s = techSpecIndexes.size();

	for(int i = 0; i < s; i++)
	{
		QString indexPath = cacheDirPath() + "/" + remoteHost + "/" + item->path + "/" + TECHSPEC_DIR + "/" + techSpecIndexes[i];

		if( QFile::exists(indexPath) )
		{
			techSpecUrlSent = true;
			emit techSpecAvailable( QUrl::fromLocalFile(indexPath) );
			return;
		}
	}

	if( item == rootItem )
	{
		techSpecUrlSent = true;
		emit techSpecAvailable( QUrl("about:blank") );
	} else
		sendTechSpecUrl(item->parent);
}

void BaseRemoteDataSource::loadItemLogo(Item *item)
{
	QString logoPath = cacheDirPath() + "/" + remoteHost + "/" + item->path + "/" + TECHSPEC_DIR + "/";

	if(QFile::exists(logoPath + LOGO_FILE)) {
		item->logo = QPixmap(logoPath + LOGO_FILE);
		item->showText = false;
	} else if(QFile::exists(logoPath + LOGO_TEXT_FILE)) {
		item->logo = QPixmap(logoPath + LOGO_TEXT_FILE);
		item->showText = true;
	}
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
