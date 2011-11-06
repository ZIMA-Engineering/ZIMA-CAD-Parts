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
		PROE=0,
		CATIA,
		IGES,
		STEP,
		STL,
		TYPES_COUNT,
		UNDEFINED
	};

	File():openFtpFile(0),isChecked(false),type(UNDEFINED),bytesDone(0){}
	QPixmap icon();

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
	BaseDataSource* server;

	Item            *parent;
	QList<Item*>    children;
	QList<File*> files;
};

#endif // ITEM_H
