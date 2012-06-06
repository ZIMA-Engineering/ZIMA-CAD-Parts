/*
  ZIMA-Parts
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

Metadata::Metadata(QString metadataPath)
{
	metadata = new QSettings(metadataPath, QSettings::IniFormat);
	metadata->setIniCodec("utf-8");

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

	metadata->beginGroup("params");
	{
		metadata->beginGroup(lang);
		{
			foreach(QString col, metadata->childKeys())
			{
				if(colRx.exactMatch(col))
				{
					columnLabels << metadata->value(col).toString();
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
	return metadata->value(QString("%1/%2").arg(part.section('.', 0, 0)).arg(col), QString()).toString();
}

void Metadata::refresh()
{
	metadata->sync();
	columnLabels.clear();
	label.clear();
	lang.clear();

	probeMetadata();
}

void Metadata::retranslate()
{
	refresh();
}

void Metadata::probeMetadata()
{
	QString currentlang = MainWindow::getCurrentMetadataLanguageCode().left(2);

	metadata->beginGroup("params");
	{
		foreach(QString group, metadata->childGroups())
		{
			if(group == currentlang)
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
