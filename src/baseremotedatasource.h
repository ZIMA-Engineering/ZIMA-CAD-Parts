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

#ifndef BASEREMOTEDATASOURCE_H
#define BASEREMOTEDATASOURCE_H

#include "basedatasource.h"

class Item;
struct File;

class BaseRemoteDataSource : public BaseDataSource
{
	Q_OBJECT
public:
	virtual QString internalName() = 0;
	virtual QIcon itemIcon(Item *item);
	virtual QIcon dataSourceIcon();
	static QString cacheDirPath();
	QString pathToDataRoot();
	QString getTechSpecPathForItem(Item *item);
	QString getPathForItem(Item *item);
	QString getRelativePathForItem(Item *item);
	QString name();

	// FIXME: make protected
	QString remoteHost, remoteLogin, remotePassword, remoteBaseDir;
	int     remotePort;

public slots:
	void sendTechSpecUrl(Item* item);
	virtual void loadRootItem(Item *item) = 0;
	virtual void loadDirectory(Item* item) = 0;
	virtual void addFileToDownload(File *f) = 0;
	virtual void downloadFiles(QList<File*> files, QString dir) = 0;
	virtual void downloadFile(File* file) = 0;
	virtual void abort() = 0;
	virtual void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings);
protected:
	void loadItemLogo(Item *item);
	void checkAndSendTechSpecUrl(Item *item);

	Item *techSpecItem;
	bool techSpecUrlSent;
	bool techSpecFilesUpdated;

};

#endif // BASEREMOTEDATASOURCE_H
