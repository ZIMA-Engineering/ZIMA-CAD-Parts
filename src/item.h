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

#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QPixmap>
#include <QFile>
#include <QList>
#include <QVariant>
#include <QDateTime>
#include <QSettings>

#include "basedatasource.h"
#include "metadata.h"
#include "thumbnail.h"
#include "downloadmodel.h"

class BaseDataSource;
class ServersModel;
class Item;
class DataTransfer;

struct File
{
	enum FileTypes {
		// Pro/e
		PRT_PROE=0,
		ASM,
		DRW,
		FRM,
		NEU_PROE,
		// CATIA
		CATPART,
		CATPRODUCT,
		CATDRAWING,
		// NX
		PRT_NX,
		// SolidWorks
		SLDPRT,
		SLDASM,
		SLDDRW,
		// Solid Edge
		PAR,
		PSM,
//		ASM,
		DFT,
		// Inventor
		IPT,
		IAM,
		IDW,
		// CAD NEUTRAL
		STEP,
		IGES,
		DWG,
		DXF,
		// NonCAD
		STL,
		BLEND,
		PDF,
		TYPES_COUNT,
		UNDEFINED
	};

    File():thumbnail(0),openFtpFile(0),isChecked(false),type(UNDEFINED),bytesDone(0),version(0),newestVersion(false),transfer(0){}
	void setName(QString name);
	QString baseName();
	void detectFileType();
	QPixmap icon();
	static QString getInternalNameForFileType(FileTypes type);
	static QString getLabelForFileType(FileTypes type);
	static QString getRxForFileType(FileTypes type);

	Item* parentItem;
	QString m_baseName; // name without version
	QString name;
	QString path;
	QString targetPath;
	Thumbnail *thumbnail;
	QList<Thumbnail*> thumbnails;
	QFile *openFtpFile;
	bool isChecked;
	QDateTime lastModified;
	FileTypes type;
	qint64 bytesDone;
	qint64 size;
	int version;
	bool newestVersion;
	DataTransfer *transfer;
	DownloadModel::TransferHandlerType transferHandler;

private:
	QPixmap m_icon;
};

class Item
{
public:
	Item();
	int row() const;
	void setNotEmpty();
	Item *child(int r);
	~Item();
	QString getLabel();
	QString pathRelativeToDataSource();
	void addThumbnail(Thumbnail *thumb);
	QList<Thumbnail*> thumbnails(bool include = true);

	int     id;
	QString name;
	QString part;
	QPixmap pixmap;
	QString pixmapFile;
	QPixmap logo;
	bool showText;
	QString path;
	QMap<QString, QVariant> params;
	QFile   *openFtpFile;
	bool    isChecked;
	bool    isDir;
	bool    isEmpty;
	bool isServer;
	bool hasTechSpecs;
	bool hasLoadedChildren;
	BaseDataSource* server;
	QList<Thumbnail*> m_thumbnails;

	Item            *parent;
	QList<Item*>    children;
	QList<File*> files;

	Metadata *metadata;
};

#endif // ITEM_H
