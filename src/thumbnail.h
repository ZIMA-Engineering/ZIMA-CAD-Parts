#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include "item.h"

#if 0
class Thumbnail
{
public:
	Thumbnail(Item *item, QString name);
	QString& name();
	QString& fullName();
	bool isLocalized() const;
	QString& language();
	QString absolutePath();
	QPixmap& pixmap();
	QPixmap& scaledPixmap(int width);
	bool isReady() const;
	void setReady(bool ready);

private:
	Item *m_item;
	QString m_name;
	QString m_fullName;
	QString m_lang;
	QPixmap m_pixmap;
	QPixmap m_scaledPixmap;
	bool m_ready;
};
#endif

#endif // THUMBNAIL_H
