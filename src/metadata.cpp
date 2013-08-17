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
#include <QDebug>

#include "metadata.h"
#include "mainwindow.h"

Metadata::Metadata(QString metadataPath, QObject *parent) : QObject(parent)
{
	metadataFile = metadataPath;

	openMetadata();

	currentAppLang = MainWindow::getCurrentMetadataLanguageCode().left(2);

	probeMetadata();
}

Metadata::~Metadata()
{
	delete metadata;
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
	else return anyVal.isEmpty() ? metadata->value(QString("%1/%2").arg(partGroup).arg(col), QString()).toString() : anyVal;
}

void Metadata::deletePart(QString part)
{
	QString grp = part.section('.', 0, 0);

	if(grp.isEmpty())
		return;

	metadata->remove(grp);
}

void Metadata::refresh()
{
	columnLabels.clear();
	label.clear();
	lang.clear();

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
}
