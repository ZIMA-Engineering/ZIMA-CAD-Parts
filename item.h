#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QPixmap>
#include <QFile>
#include <QList>
#include <QVariant>
#include "ftpserver.h"

class FtpServer;
class Item;

struct File
{
	File():isChecked(false){}

	Item* parentItem;
	QString name;
	QString path;
	QString pixmapPath;
	QPixmap pixmap;
	QFile *openFtpFile;
	bool isChecked;
	QDateTime lastModified;
};

class Item
{
public:
	Item();
	int row() const;
	int nonEmptyRow() const;
	int nonEmptyChildrenNum();
	void setNotEmpty();
	Item *child(int r);
	Item *nonEmptyChild(int r);
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
	FtpServer* server;

	Item            *parent;
	QList<Item*>    children;
	QList<File*> files;
};

#endif // ITEM_H
