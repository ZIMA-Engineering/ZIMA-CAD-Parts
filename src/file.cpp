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

#include "file.h"

#include <QFileIconProvider>


FileTypeList File::versionedTypes()
{
	return FileTypeList() << FileType::PRT_PROE
	       << FileType::ASM
	       << FileType::DRW
	       << FileType::FRM
	       << FileType::NEU_PROE;
}

QString File::getInternalNameForFileType(FileType::FileType type)
{
	switch(type)
	{
	case FileType::PRT_PROE:
		return "prt_proe";
	case FileType::ASM:
		return "asm";
	case FileType::DRW:
		return "drw";
	case FileType::FRM:
		return "frm";
	case FileType::NEU_PROE:
		return "neu_proe";
	case FileType::CATPART:
		return "catpart";
	case FileType::CATPRODUCT:
		return "catproduct";
	case FileType::CATDRAWING:
		return "catdrawing";
	case FileType::PRT_NX:
		return "prt_nx";
	case FileType::SLDPRT:
		return "sldprt";
	case FileType::SLDASM:
		return "sldasm";
	case FileType::SLDDRW:
		return "slddrw";
	case FileType::PAR:
		return "par";
	case FileType::PSM:
		return "psm";
	case FileType::DFT:
		return "dft";
	case FileType::IPT:
		return "ipt";
	case FileType::IAM:
		return "iam";
	case FileType::IDW:
		return "idw";
	case FileType::DWG:
		return "dwg";
	case FileType::STEP:
		return "step";
	case FileType::IGES:
		return "iges";
	case FileType::DXF:
		return "dxf";
	case FileType::STL:
		return "stl";
	case FileType::BLEND:
		return "blend";
	case FileType::PDF:
		return "pdf";
	case FileType::OFFICE_WRITER:
		return "office-document";
	case FileType::OFFICE_CALC:
		return "office-spreadsheet";
	case FileType::OFFICE_IMPRESS:
		return "office-presentation";
	case FileType::OFFICE_DRAW:
		return "office-drawing";
	case FileType::OFFICE_PROJECT:
		return "office-project";
	case FileType::OFFICE_BASE:
		return "office-database";
    case FileType::OFFICE_EML:
        return "internet-mail";
	case FileType::OFFICE_MBOX:
		return "mail-mbox";
	case FileType::ZIP:
		return "zip";
	case FileType::RAR:
		return "rar";
	case FileType::TAR:
		return "tar";
	case FileType::ZIP7:
		return "7z";
    case FileType::FILE_IMAGE:
        return "image-x-generic";
	default:
		return "undefined";
	}
}

QString File::getLabelForFileType(FileType::FileType type)
{
	switch(type)
	{
	case FileType::PRT_PROE:
		return "*.prt";
	case FileType::ASM:
		return "*.asm";
	case FileType::DRW:
		return "*.drw";
	case FileType::FRM:
		return "*.frm";
	case FileType::NEU_PROE:
		return "*.neu";
	case FileType::CATPART:
		return "*.catpart";
	case FileType::CATPRODUCT:
		return "*.catproduct";
	case FileType::CATDRAWING:
		return "*.catdrawing";
	case FileType::PRT_NX:
		return "*.prt";
	case FileType::SLDPRT:
		return "*.sldprt";
	case FileType::SLDASM:
		return "*.sldasm";
	case FileType::SLDDRW:
		return "*.slddrw";
	case FileType::PAR:
		return "*.par";
	case FileType::PSM:
		return "*.psm";
	case FileType::DFT:
		return "*.dft";
	case FileType::IPT:
		return "*.ipt";
	case FileType::IAM:
		return "*.iam";
	case FileType::IDW:
		return "*.idw";
	case FileType::DWG:
		return "*.dwg";
	case FileType::STEP:
		return "*.step";
	case FileType::IGES:
		return "*.iges";
	case FileType::DXF:
		return "*.dxf";
	case FileType::STL:
		return "*.stl";
	case FileType::BLEND:
		return "*.blend";
	case FileType::PDF:
		return "*.pdf";
	case FileType::OFFICE_WRITER:
		return QObject::tr("Office document");
	case FileType::OFFICE_CALC:
		return QObject::tr("Office spreadsheet");
	case FileType::OFFICE_IMPRESS:
		return QObject::tr("Office presentation");
	case FileType::OFFICE_DRAW:
		return QObject::tr("Office drawing");
	case FileType::OFFICE_PROJECT:
		return QObject::tr("Office project");
	case FileType::OFFICE_BASE:
		return QObject::tr("Office database");
    case FileType::OFFICE_EML:
        return "*.eml";
	case FileType::OFFICE_MBOX:
		return "*.mbox";
	case FileType::ZIP:
		return "*.zip";
	case FileType::RAR:
		return "*.rar";
	case FileType::TAR:
		return "*.tar.*";
	case FileType::ZIP7:
		return "*.7z";
    case FileType::FILE_IMAGE:
        return QObject::tr("Image/Picture");
	default:
		return "undefined";
	}
}

QString File::getRxForFileType(FileType::FileType type)
{
	switch(type)
	{
	case FileType::PRT_PROE:
		return "((^.+\\.prt)\\.(\\d+)$)";
	case FileType::ASM:
		return "((^.+\\.asm)\\.(\\d+)$)";
	case FileType::DRW:
		return "((^.+\\.drw)\\.(\\d+)$)";
	case FileType::FRM:
		return "((^.+\\.frm)\\.(\\d+)$)";
	case FileType::NEU_PROE:
		return "((^.+\\.neu)\\.(\\d+)$)";
	case FileType::CATPART:
		return "(^.+\\.catpart$)";
	case FileType::CATPRODUCT:
		return "(^.+\\.catproduct$)";
	case FileType::CATDRAWING:
		return "(^.+\\.catdrawing$)";
	case FileType::PRT_NX:
		return "(^.+\\.prt$)";
	case FileType::SLDPRT:
		return "(^.+\\.sldprt$)";
	case FileType::SLDASM:
		return "(^.+\\.sldasm$)";
	case FileType::SLDDRW:
		return "(^.+\\.slddrw$)";
	case FileType::PAR:
		return "(^.+\\.par$)";
	case FileType::PSM:
		return "(^.+\\.psm$)";
	case FileType::DFT:
		return "(^.+\\.dft$)";
	case FileType::IPT:
		return "(^.+\\.ipt$)";
	case FileType::IAM:
		return "(^.+\\.iam$)";
	case FileType::IDW:
		return "(^.+\\.idw$)";
	case FileType::DWG:
		return "(^.+\\.dwg$)";
	case FileType::STEP:
		return "(^.+\\.step|.+\\.stp$)";
	case FileType::IGES:
		return "(^.+\\.iges|.+\\.igs$)";
	case FileType::DXF:
		return "(^.+\\.dxf$)";
	case FileType::STL:
		return "(^.+\\.stl$)";
	case FileType::BLEND:
		return "(^.+\\.blend$)";
	case FileType::PDF:
		return "(^.+\\.pdf$)";
	case FileType::OFFICE_WRITER:
		return getRxFromStringList(QStringList() << "odt" << "ott" << "odm" << "doc" << "dot" << "docx" << "docm" << "dotx" << "dotm");
	case FileType::OFFICE_CALC:
		return getRxFromStringList(QStringList() << "ods" << "ots" << "xls" << "xlt" << "xlm" << "xlsx" << "xlsm" << "xltx" << "xltm" << "csv");
	case FileType::OFFICE_IMPRESS:
		return getRxFromStringList(QStringList() << "odp" << "otp" << "ppt" << "pot" << "pps" << "pptx" << "pptm" << "potx" << "potm" << "ppam" << "ppsx" << "ppsm" << "sldx" << "sldm");
	case FileType::OFFICE_DRAW:
		return getRxFromStringList(QStringList() << "odg" << "otg");
	case FileType::OFFICE_PROJECT:
		return getRxFromStringList(QStringList() << "mpd" << "mpp");
	case FileType::OFFICE_BASE:
		return getRxFromStringList(QStringList() << "odb" << "mdb" << "accdb" << "accde" << "accdt" << "accdr");
    case FileType::OFFICE_EML:
        return "(^.+\\.eml$)";
	case FileType::OFFICE_MBOX:
		return "(^.+\\.mbox$)";
	case FileType::ZIP:
		return "(^.+\\.zip$)";
	case FileType::RAR:
		return "(^.+\\.rar$)";
	case FileType::TAR:
		return "(^.+\\.tar(\\.[^\\.]+)?$)";
	case FileType::ZIP7:
		return "(^.+\\.7z$)";
    case FileType::FILE_IMAGE:
        return getRxFromStringList(QStringList() << "png" << "jpg" << "jpeg");
	default:
		return "";
	}
}

QString File::getRxFromStringList(const QStringList &extensions)
{
	QString ret;
	foreach(QString i, extensions)
	{
		ret += QString("(^.+\\.%1)|").arg(i);
	}
	ret.chop(1);
	return ret;
}

FileMetadata::FileMetadata(const QString &path)
    : type(FileType::UNDEFINED)
{
    fileInfo = QFileInfo(path);
    detectFileType();
}

FileMetadata::FileMetadata(const QFileInfo &fi)
	: type(FileType::UNDEFINED),
	  fileInfo(fi)
{
	detectFileType();
}

FileMetadata::~FileMetadata()
{
}

void FileMetadata::detectFileType()
{
	for(int i = 0; i < FileType::TYPES_COUNT; i++)
	{
		QRegExp rx(File::getRxForFileType((FileType::FileType)i));
		// *.dxf vs. *.DXF in some examples
		rx.setCaseSensitivity(Qt::CaseInsensitive);

		if(rx.exactMatch(fileInfo.fileName()))
		{
			type = (FileType::FileType)i;
			break;
		}
	}
}
