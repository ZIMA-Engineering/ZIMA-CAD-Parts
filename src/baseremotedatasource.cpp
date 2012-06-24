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

#include "baseremotedatasource.h"
#include "mainwindow.h"

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

	QStringList filters;
	filters << "index_??.html" << "index_??.htm" << "index.html" << "index.htm";

	QDir dir(getTechSpecPathForItem(item));
	QStringList indexes = dir.entryList(filters, QDir::Files | QDir::Readable);

	if(indexes.isEmpty())
	{
		if( item == rootItem )
		{
			techSpecUrlSent = true;
			emit techSpecAvailable(QUrl("about:blank"));
		} else
			checkAndSendTechSpecUrl(item->parent);

		return;
	}

	QString selectedIndex = indexes.first();
	indexes.removeFirst();

	foreach(QString index, indexes)
	{
		QString prefix = index.section('.', 0, 0);

		if(prefix.lastIndexOf('_') == prefix.count()-3 && prefix.right(2) == currentMetadataLang)
			selectedIndex = index;
	}

	techSpecUrlSent = true;
	emit techSpecAvailable(QUrl::fromLocalFile(dir.path() + "/" + selectedIndex));
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

QString BaseRemoteDataSource::getTechSpecPathForItem(Item *item)
{
	return getPathForItem(item) + "/" + TECHSPEC_DIR;
}

QString BaseRemoteDataSource::getPathForItem(Item *item)
{
	return cacheDirPath() + "/" + remoteHost + "/" + item->path;
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
