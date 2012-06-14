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

#ifndef METADATA_H
#define METADATA_H

#include <QObject>
#include <QSettings>
#include <QStringList>

class Metadata : public QObject
{
	Q_OBJECT
public:
	explicit Metadata(QString metadataPath, QObject *parent = 0);
	~Metadata();
	QString getLabel();
	QStringList getColumnLabels();
	QString getPartParam(QString part, int col);

public slots:
	void refresh();
	void retranslate(QString lang = QString());

private:
	void probeMetadata();

	QSettings *metadata;
	QString currentAppLang;
	QString lang;
	QStringList columnLabels;
	QString label;

signals:
	void retranslated();
};

#endif // METADATA_H
