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
#include "mainwindow.h"
#include "item.h"

Metadata::Metadata(Item *item, QObject *parent)
	: QObject(parent),
	  m_item(item),
	  m_loadedIncludes(0),
	  m_includedData(false)
{
	metadataFile = m_item->server->getPathForItem(m_item) + "/" + TECHSPEC_DIR + "/" + METADATA_FILE;

	currentAppLang = MainWindow::getCurrentMetadataLanguageCode().left(2);
}

Metadata::~Metadata()
{
	emit includeRequireCancelled(m_item);

	delete metadata;
}

void Metadata::init()
{
	openMetadata();
	probeMetadata();
}

QString Metadata::getLabel()
{
	if(label.count())
		return label;

	return (label = metadata->value(QString("params/%1/label").arg(lang), QString()).toString());
}

QStringList Metadata::getColumnLabels()
{
	if(columnLabels.count())
		return columnLabels;

	QRegExp colRx("^\\d+$");
	int colIndex = 1;

	metadata->beginGroup("params");
	{
		metadata->beginGroup(lang);
		{
			foreach(QString col, metadata->childKeys())
			{
				if(colRx.exactMatch(col))
				{
					//columnLabels << metadata->value(col).toString();

					columnLabels << metadata->value( QString("%1").arg(colIndex++) ).toString();
				}
			}
		}
		metadata->endGroup();
	}
	metadata->endGroup();

	foreach(Metadata *include, includes)
		columnLabels << include->getColumnLabels();

	return columnLabels;
}

QString Metadata::getPartParam(QString part, int col)
{
	QString partGroup = part.section('.', 0, 0);
	QString anyVal;
	QString val;

	metadata->beginGroup(partGroup);

	foreach(QString group, metadata->childGroups())
	{
		if(!(val = metadata->value(QString("%1/%2").arg(group).arg(col)).toString()).isEmpty() && group == currentAppLang)
			break;

		if(anyVal.isEmpty())
			anyVal = val;
	}

	metadata->endGroup();

	if(!val.isEmpty())
		return val;
	else {

		QString ret = anyVal.isEmpty() ? metadata->value(QString("%1/%2").arg(partGroup).arg(col), QString()).toString() : anyVal;

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

	metadata->remove(grp);
}

QList<Item*> Metadata::includedThumbnailItems()
{
	return m_thumbItems;
}

void Metadata::refresh()
{
	columnLabels.clear();
	label.clear();
	lang.clear();
	m_includeHash.clear();
	m_thumbItems.clear();

	delete metadata;

	openMetadata();
	probeMetadata();
}

void Metadata::retranslate(QString lang)
{
	if(lang.isEmpty())
		currentAppLang = MainWindow::getCurrentMetadataLanguageCode().left(2);
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

		QString path = QDir::cleanPath(m->m_item->server->name() + m->m_item->pathRelativeToDataSource());

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

			m_item->server->assignThumbnailsToFiles(m_item, thumbs);
		}
	}
}

void Metadata::openMetadata()
{
	metadata = new QSettings(metadataFile, QSettings::IniFormat);
	metadata->setIniCodec("utf-8");
}

void Metadata::probeMetadata()
{
	metadata->beginGroup("params");
	{
		foreach(QString group, metadata->childGroups())
		{
			if(group == currentAppLang)
			{
				lang = group;
				break;
			}
		}

		if(lang.isEmpty())
		{
			QStringList childGroups = metadata->childGroups();

			if(childGroups.contains("en"))
				lang = "en";
			else if(childGroups.count())
				lang = childGroups.first();
			else {
				metadata->endGroup();
				return;
			}
		}
	}
	metadata->endGroup();

	metadata->beginGroup("include");
	{
		QStringList toInclude;
		QStringList data = buildIncludePaths(metadata->value("data").toStringList());
		QStringList thumbs = buildIncludePaths(metadata->value("thumbnails").toStringList());

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
	metadata->endGroup();
}

QString Metadata::buildIncludePath(QString raw)
{
	QString dsName = m_item->server->name();
	QString ret;

	raw = raw.trimmed();

	if(raw.startsWith('/'))
		ret = QDir::cleanPath(dsName + raw);
	else
		ret = QDir::cleanPath(dsName + m_item->pathRelativeToDataSource() + "/" + raw);

	if(!ret.startsWith(dsName + "/"))
		ret = dsName + "/" + ret;

	return ret;
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
