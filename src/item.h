/*
  ZIMA-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011 Jakub Skokan <aither@havefun.cz>

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

class BaseDataSource;
class Item;

struct File
{
	enum FileTypes {
		// Pro/e
		PRT_PROE=0,
		ASM,
		DRW,
		FRM,
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
		DWG,
		// CAD NEUTRAL
		STEP,
		IGES,
//		DWG,
		DXF,
		// NonCAD
		STL,
		BLEND,
		TYPES_COUNT,
		UNDEFINED
	};

	File():openFtpFile(0),isChecked(false),type(UNDEFINED),bytesDone(0){}
	void setName(QString name);
	void detectFileType();
	QPixmap icon();
	static QString getInternalNameForFileType(FileTypes type);
	static QString getLabelForFileType(FileTypes type);
	static QString getRxForFileType(FileTypes type);

	Item* parentItem;
	QString name;
	QString path;
	QString targetPath;
	QString pixmapPath;
	QPixmap pixmap;
	QPixmap scaledThumb;
	QFile *openFtpFile;
	bool isChecked;
	QDateTime lastModified;
	FileTypes type;
	qint64 bytesDone;
	qint64 size;
};

class Item
{
public:
	Item();
	int row() const;
	void setNotEmpty();
	Item *child(int r);
	~Item();

	int     id;
	QString name;
	QString part;
	QPixmap pixmap;
	QString pixmapFile;
	QString path;
	QMap<QString, QVariant> params;
	QFile   *openFtpFile;
	bool    isChecked;
	bool    isDir;
	bool    isEmpty;
	bool isServer;
	BaseDataSource* server;

	Item            *parent;
	QList<Item*>    children;
	QList<File*> files;
};

#endif // ITEM_H
