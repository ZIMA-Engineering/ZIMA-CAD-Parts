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
#include <QFileInfo>

#include "metadata.h"

//class LocalDataSource;
//class ServersModel;
//class Item;
//class DataTransfer;
class Thumbnail;

class File
{

public:
    enum FileType {
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
	    OFFICE_WRITER,
	    OFFICE_CALC,
	    OFFICE_IMPRESS,
	    OFFICE_DRAW,
	    OFFICE_PROJECT,
	    OFFICE_BASE,
	    TYPES_COUNT,
	    UNDEFINED
	};

    File(const QFileInfo &fi);

    FileType type;
    int version;
    bool newestVersion;
    QFileInfo fileInfo;

    QPixmap icon() const;

    static QString getInternalNameForFileType(FileType type);
    static QString getLabelForFileType(FileType type);
    static QString getRxForFileType(FileType type);
    static QString getRxFromStringList(const QStringList &extensions);

private:
    void detectFileType();
};

#if 0
class Item
{
public:
	Item();
	int row() const;
	void setNotEmpty();
	Item *child(int r);
	~Item();
	QString getLabel();
	void addThumbnail(Thumbnail *thumb);
	QList<Thumbnail*> thumbnails(bool include = true);

	QString name;
	QPixmap logo;
	bool showText;
	QString path;
	bool    isDir;
	bool isServer;
    //LocalDataSource* server;

	Item            *parent;
	QList<Item*>    children;
	QList<File*> files;

	Metadata *metadata;

private:
    QList<Thumbnail*> m_thumbnails;
};
#endif

#endif // ITEM_H
