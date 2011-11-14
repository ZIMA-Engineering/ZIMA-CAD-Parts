#include "item.h"

QPixmap File::icon()
{
	if( type == File::UNDEFINED )
	{
		QRegExp rx(".+\\.prt\\.\\d+");

		if( rx.exactMatch(name) )
			type = File::PROE;
	}

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

int Item::nonEmptyRow() const
{
	if (parent)
	{
		int i = 0;
		foreach(Item *p, parent->children)
		{
			if (!p->isDir || !p->isEmpty)
				i++;

			if (p == this)
				return i;
		}
	}

	return 0;
}

int Item::nonEmptyChildrenNum()
{
	int i = 1;
	foreach(Item *p, children)
	{
		if (!p->isDir || !p->isEmpty)
			i++;
	}
	return isDir ? i : 0;
}

Item *Item::child(int r)
{
	return children.value(r);
}

Item *Item::nonEmptyChild(int r)
{
	int i = 0;
	foreach(Item *p, children)
	{
		if (!p->isDir || !p->isEmpty)
		{
			i++;
			if (i == r)
				return p;
		}
	}
	return 0;
}

void Item::setNotEmpty()
{
	isEmpty = false;
	if (parent)
		parent->setNotEmpty();
}
