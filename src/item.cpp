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

	QRegExp rx(".+\\.prt\\.\\d+");

	if( rx.exactMatch(name) )
		type = File::PROE;
}

QPixmap File::icon()
{
	if( type == File::UNDEFINED )
		detectFileType();

	switch( type )
	{
	case File::PROE:
		return QPixmap(":/gfx/icons/prt.png");
	default:
		return QPixmap();
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
