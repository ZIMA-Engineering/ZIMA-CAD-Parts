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

#include <QRegExp>
#include <QDir>
#include <QDebug>

#include "metadata.h"
#include "settings.h"
#include "item.h"

#if 0
Metadata::Metadata(const QString &path, QObject *parent)
	: QObject(parent),
      m_path(path),
	  m_loadedIncludes(0),
	  m_includedData(false)
{
    m_settings = new QSettings(m_path + "/" + TECHSPEC_DIR + "/" + METADATA_FILE, QSettings::IniFormat);
    m_settings->setIniCodec("utf-8");

	currentAppLang = Settings::get()->getCurrentLanguageCode().left(2);

    m_settings->beginGroup("params");
    {
        foreach(QString group, m_settings->childGroups())
        {
            if(group == currentAppLang)
            {
                lang = group;
                break;
            }
        }

        if(lang.isEmpty())
        {
            QStringList childGroups = m_settings->childGroups();

            if(childGroups.contains("en"))
                lang = "en";
            else if(childGroups.count())
                lang = childGroups.first();
            else {
                m_settings->endGroup();
                return;
            }
        }
    }
    m_settings->endGroup();

    m_settings->beginGroup("include");
    {
        QStringList toInclude;
        QStringList data = buildIncludePaths(m_settings->value("data").toStringList());
        QStringList thumbs = buildIncludePaths(m_settings->value("thumbnails").toStringList());

        setIncludeMark(data, IncludeMetadata);
        setIncludeMark(thumbs, IncludeThumbnails);

        toInclude << data << thumbs;

        toInclude.removeDuplicates();

        if(data.isEmpty())
            emit ready(m_item);
        else
            m_includedData = true;

        foreach(QString path, toInclude)
        emit includeRequired(m_item, path);
    }
    m_settings->endGroup();
}

Metadata::~Metadata()
{
	if(m_includeHash.size())
		emit includeRequireCancelled(m_item);

    delete m_settings;

    columnLabels.clear();
    label.clear();
    lang.clear();
    m_includeHash.clear();
    m_thumbItems.clear();
}

QString Metadata::getLabel()
{
	if(label.count())
		return label;

    return (label = m_settings->value(QString("params/%1/label").arg(lang), QString()).toString());
}

QStringList Metadata::getColumnLabels()
{
	if(columnLabels.count())
		return columnLabels;

	QRegExp colRx("^\\d+$");
	int colIndex = 1;

    m_settings->beginGroup("params");
	{
        m_settings->beginGroup(lang);
		{
            foreach(QString col, m_settings->childKeys())
			{
				if(colRx.exactMatch(col))
				{
					//columnLabels << metadata->value(col).toString();

                    columnLabels << m_settings->value( QString("%1").arg(colIndex++) ).toString();
				}
			}
		}
        m_settings->endGroup();
	}
    m_settings->endGroup();

	foreach(Metadata *include, includes)
	columnLabels << include->getColumnLabels();

	return columnLabels;
}

QString Metadata::getPartParam(QString part, int col)
{
	QString partGroup = part.section('.', 0, 0);
	QString anyVal;
	QString val;

    m_settings->beginGroup(partGroup);

    foreach(QString group, m_settings->childGroups())
	{
        if(!(val = m_settings->value(QString("%1/%2").arg(group).arg(col)).toString()).isEmpty() && group == currentAppLang)
			break;

		if(anyVal.isEmpty())
			anyVal = val;
	}

    m_settings->endGroup();

	if(!val.isEmpty())
		return val;
	else {

        QString ret = anyVal.isEmpty() ? m_settings->value(QString("%1/%2").arg(partGroup).arg(col), QString()).toString() : anyVal;

		if(ret.isEmpty())
		{
			foreach(Metadata *include, includes)
			{
				QString tmp = include->getPartParam(part, col);

				if(!tmp.isEmpty())
					return tmp;
			}
		}

		return ret;
	}
}

void Metadata::deletePart(QString part)
{
	QString grp = part.section('.', 0, 0);

	if(grp.isEmpty())
		return;

    m_settings->remove(grp);
}

QList<Item*> Metadata::includedThumbnailItems()
{
	return m_thumbItems;
}

void Metadata::retranslate(QString lang)
{
	if(lang.isEmpty())
		currentAppLang = Settings::get()->getCurrentLanguageCode().left(2);
	else
		currentAppLang = lang;

	refresh();

	emit retranslated();
}

void Metadata::provideInclude(Metadata *m, QString path)
{
	m_loadedIncludes++;

	if(!m)
	{
		if(path.isEmpty())
			qDebug() << "Unable to include" << path << ": this directory does not have metadata";
		else
			qDebug() << "Unable to include" << path << ": path does not exists";
	} else {
		qDebug() << "Include loaded" << m->getLabel();

        QString path = QDir::cleanPath(m->m_item->path);

		QHashIterator<QString, Include> i(m_includeHash);
		while(i.hasNext())
		{
			i.next();

			if(i.key() != path)
			{
				qDebug() << i.key() << "!=" << path;
				continue;
			}

			Include inc = i.value();

			if((inc & IncludeMetadata) == IncludeMetadata )
				includes << m;

			if((inc & IncludeThumbnails) == IncludeThumbnails)
				m_thumbItems << m->m_item;
		}
	}

	if(m_loadedIncludes == includes.size())
	{
		if(m_includedData)
			emit ready(m_item);

		if(!m_thumbItems.isEmpty())
		{
			QList<Thumbnail*> thumbs;

			foreach(Item *it, m_thumbItems)
			thumbs << it->thumbnails(false);

#warning todo			m_item->server->assignThumbnailsToFiles(m_item, thumbs);
		}
	}
}

QString Metadata::buildIncludePath(QString raw)
{
#warning todo
#if 0
	QString dsName = m_item->server->name();
	QString ret;

	raw = raw.trimmed();

	if(raw.startsWith('/'))
		ret = QDir::cleanPath(dsName + raw);
	else
		ret = QDir::cleanPath(dsName + m_item->server->getRelativePathForItem(m_item) + "/" + raw);

	if(!ret.startsWith(dsName + "/"))
		ret = dsName + "/" + ret;

	return ret;
#endif
}

QStringList Metadata::buildIncludePaths(QStringList raw)
{
	QStringList ret;

	foreach(QString s, raw)
	ret << buildIncludePath(s);

	return ret;
}

void Metadata::setIncludeMark(QStringList &list, Include mark)
{
	foreach(QString path, list)
	{
		if(!m_includeHash.contains(path))
			m_includeHash[path] = IncludeNothing;

		m_includeHash[path] = (Include) (m_includeHash[path] | mark);
	}
}
#endif
