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

#include "item.h"
#include <QDebug>

void File::setName(QString name)
{
	this->name = name;

	detectFileType();
}

void File::detectFileType()
{
	type = File::UNDEFINED;

	for(int i = 0; i < TYPES_COUNT; i++)
	{
		QRegExp rx(getRxForFileType((File::FileTypes)i));

		if(rx.exactMatch(name))
		{
			type = (File::FileTypes)i;
			break;
		}
	}
}

QPixmap File::icon()
{
	if( type == File::UNDEFINED )
		detectFileType();

	switch( type )
	{
	case File::PRT_PROE:
	case File::PRT_NX:
		return QPixmap(":/gfx/icons/prt.png");
	default:
		return QPixmap();
	}
}

QString File::getInternalNameForFileType(File::FileTypes type)
{
	switch(type)
	{
	case File::PRT_PROE:
		return "prt_proe";
	case File::ASM:
		return "asm";
	case File::DRW:
		return "drw";
	case File::CATPART:
		return "catpart";
	case File::CATPRODUCT:
		return "catproduct";
	case File::CATDRAWING:
		return "catdrawing";
	case File::PRT_NX:
		return "prt_nx";
	case File::SLDPRT:
		return "sldprt";
	case File::SLDASM:
		return "sldasm";
	case File::SLDDRW:
		return "slddrw";
	case File::PAR:
		return "par";
	case File::PSM:
		return "psm";
	case File::DFT:
		return "dft";
	case File::IPT:
		return "ipt";
	case File::IAM:
		return "iam";
	case File::DWG:
		return "dwg";
	case File::STEP:
		return "step";
	case File::IGES:
		return "iges";
	case File::DXF:
		return "dxf";
	case File::STL:
		return "stl";
	case File::BLEND:
		return "blend";
	default:
		return "undefined";
	}
}

QString File::getLabelForFileType(File::FileTypes type)
{
	switch(type)
	{
	case File::PRT_PROE:
		return "*.prt";
	case File::ASM:
		return "*.asm";
	case File::DRW:
		return "*.drw";
	case File::CATPART:
		return "*.catpart";
	case File::CATPRODUCT:
		return "*.catproduct";
	case File::CATDRAWING:
		return "*.catdrawing";
	case File::PRT_NX:
		return "*.prt";
	case File::SLDPRT:
		return "*.sldprt";
	case File::SLDASM:
		return "*.sldasm";
	case File::SLDDRW:
		return "*.slddrw";
	case File::PAR:
		return "*.par";
	case File::PSM:
		return "*.psm";
	case File::DFT:
		return "*.dft";
	case File::IPT:
		return "*.ipt";
	case File::IAM:
		return "*.iam";
	case File::DWG:
		return "*.dwg";
	case File::STEP:
		return "*.step";
	case File::IGES:
		return "*.iges";
	case File::DXF:
		return "*.dxf";
	case File::STL:
		return "*.stl";
	case File::BLEND:
		return "*.blend";
	default:
		return "undefined";
	}
}

QString File::getRxForFileType(File::FileTypes type)
{
	switch(type)
	{
	case File::PRT_PROE:
		return "(^.+\\.prt\\.\\d+$)";
	case File::ASM:
		return "(^.+\\.asm\\.\\d+$)";
	case File::DRW:
		return "(^.+\\.drw\\.\\d+$)";
	case File::CATPART:
		return "(^.+\\.catpart$)";
	case File::CATPRODUCT:
		return "(^.+\\.catproduct$)";
	case File::CATDRAWING:
		return "(^.+\\.catdrawing$)";
	case File::PRT_NX:
		return "(^.+\\.prt$)";
	case File::SLDPRT:
		return "(^.+\\.sldprt$)";
	case File::SLDASM:
		return "(^.+\\.sldasm$)";
	case File::SLDDRW:
		return "(^.+\\.slddrw$)";
	case File::PAR:
		return "(^.+\\.par$)";
	case File::PSM:
		return "(^.+\\.psm$)";
	case File::DFT:
		return "(^.+\\.dft$)";
	case File::IPT:
		return "(^.+\\.ipt$)";
	case File::IAM:
		return "(^.+\\.iam$)";
	case File::DWG:
		return "(^.+\\.dwg$)";
	case File::STEP:
		return "(^.+\\.step|.+\\.stp$)";
	case File::IGES:
		return "(^.+\\.iges|.+\\.igs$)";
	case File::DXF:
		return "(^.+\\.dxf$)";
	case File::STL:
		return "(^.+\\.stl$)";
	case File::BLEND:
		return "(^.+\\.blend$)";
	default:
		return "";
	}
}

Item::Item()
{
	isChecked = false;
	openFtpFile = 0;
	parent = 0;
	isDir = false;
	isServer = false;
	isEmpty = true;
	server = 0;
}

Item::~Item()
{
	delete openFtpFile;
	qDeleteAll(files);
	qDeleteAll(children);
	files.clear();
	children.clear();
}

int Item::row() const
{
	if (parent)
		return parent->children.indexOf(const_cast<Item*>(this));

	return 0;
}

Item *Item::child(int r)
{
	return children.value(r);
}


void Item::setNotEmpty()
{
	isEmpty = false;
	if (parent)
		parent->setNotEmpty();
}
