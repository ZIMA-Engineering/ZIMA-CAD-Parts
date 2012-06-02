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
#ifndef BASEDATASOURCE_H
#define BASEDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QSettings>
#include <QIcon>
#include <QApplication>

#include "item.h"

#define TECHSPEC_DIR "0000-index"
#define METADATA_FILE "metadata.ini"
#define LOGO_FILE "logo.png"
#define LOGO_TEXT_FILE "logo-text.png"

class QListWidgetItem;
class Item;
struct File;

enum DataSources {
	LOCAL=0,
	FTP,
	UNDEFINED
};

class BaseDataSource : public QObject
{
	Q_OBJECT
public:
	explicit BaseDataSource(QObject *parent = 0);
	Item *getRootItem();
	virtual QString internalName() = 0;
	virtual QIcon itemIcon(Item *item);
	virtual QIcon dataSourceIcon();

	Item    *rootItem;
	QListWidgetItem *lwItem;
	QString label;
	DataSources dataSource;
public slots:
	virtual void loadRootItem(Item *item) = 0;
	virtual void loadDirectory(Item* item) = 0;
	virtual void sendTechSpecUrl(Item* item) = 0;
	virtual void addFileToDownload(File *f) = 0;
	virtual void downloadFiles(QList<File*> files, QString dir) = 0;
	virtual void downloadFile(File* file) = 0;
	virtual void resumeDownload() = 0;
	virtual void abort() = 0;
	virtual void loadSettings(QSettings& settings);
	virtual void deleteDownloadQueue();
	void saveSettings(QSettings& settings);
protected:
	virtual void loadItemLogo(Item *item) = 0;

	QStringList techSpecIndexes;
signals:
	//void partDownloaded(Item*);
	void itemInserted(Item*);
	void updateAvailable(Item*);
	void loadingItem(Item*);
	void itemLoaded(Item*);
	void allItemsLoaded();
	void fileProgress(File*);
	void fileDownloaded(File*);
	void filesDownloaded(); //TODO: identify this somehow
	void gotThumbnail(File*);
	void statusUpdated(QString);
	void techSpecAvailable(QUrl);
	void metadataReady(Item*);
	void errorOccured(QString);
};

#endif // BASEDATASOURCE_H
