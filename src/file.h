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

#ifndef FILE_H
#define FILE_H

#include <QString>
#include <QPixmap>
#include <QFile>
#include <QList>
#include <QVariant>
#include <QDateTime>
#include <QFileInfo>

namespace FileType {

typedef enum {
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
//		FileType::ASM,
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
    OFFICE_EML,
    OFFICE_MBOX,
    // Archives
    ZIP,
    RAR,
    TAR,
    ZIP7,
    // images
    FILE_IMAGE,
    // audio
    FILE_AUDIO,
    // this must go last
    TYPES_COUNT,
    UNDEFINED
} FileType;

} // namespace FileType

typedef QList<FileType::FileType> FileTypeList;

/*! Additional metadata for CAD-aware file.
 * Used in FileModel class.
 */
class File
{

public:
    static FileTypeList versionedTypes();
    static QString getInternalNameForFileType(FileType::FileType type);
    static QString getLabelForFileType(FileType::FileType type);
    static QString getRxForFileType(FileType::FileType type);
    static QString getRxFromStringList(const QStringList &extensions);
};


class FileMetadata
{
public:
    FileMetadata(const QString &path);
    FileMetadata(const QFileInfo &fi);
    ~FileMetadata();

    //! CAD type of the file
    FileType::FileType type;

    QFileInfo fileInfo;

private:
    void detectFileType();

};

#endif // FILE_H
