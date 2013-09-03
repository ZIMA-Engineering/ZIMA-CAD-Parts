#include "thumbnail.h"

Thumbnail::Thumbnail(Item *item, QString name)
	: m_item(item),
	  m_fullName(name),
	  m_ready(true)
{
	QString prefix = m_fullName.section('.', -2, -2);

	if(prefix.lastIndexOf('_') == prefix.count()-3)
	{
		m_name = prefix.left(prefix.count()-3);
		m_lang = prefix.right(2);

	} else
		m_name = prefix;
}

QString& Thumbnail::name()
{
	return m_name;
}

QString& Thumbnail::fullName()
{
	return m_fullName;
}

bool Thumbnail::isLocalized() const
{
	return !m_lang.isEmpty();
}

QString& Thumbnail::language()
{
	return m_lang;
}

QString Thumbnail::absolutePath()
{
	return m_item->server->getPathForItem(m_item) + "/" + m_fullName;
}

QPixmap& Thumbnail::pixmap()
{
	if(m_pixmap.isNull())
		m_pixmap = QPixmap(absolutePath());

	return m_pixmap;
}

QPixmap& Thumbnail::scaledPixmap(int width)
{
	if(m_scaledPixmap.isNull() || m_scaledPixmap.width() != width)
		m_scaledPixmap = pixmap().scaledToWidth(width, Qt::SmoothTransformation);

	return m_scaledPixmap;
}

bool Thumbnail::isReady() const
{
	return m_ready;
}
void Thumbnail::setReady(bool ready)
{
	m_ready = ready;
}
