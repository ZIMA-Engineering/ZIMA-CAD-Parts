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

#ifndef METADATA_H
#define METADATA_H

#include <QObject>
#include <QStringList>
#include <QSettings>

#if 0
class Metadata : public QObject
{
	Q_OBJECT
public:
    explicit Metadata(const QString &path, QObject *parent = 0);
	~Metadata();

	QString getLabel();
	QStringList getColumnLabels();
	QString getPartParam(QString part, int col);
	void deletePart(QString part);
	QList<Item*> includedThumbnailItems();

public slots:
	void retranslate(QString lang = QString());
	void provideInclude(Metadata *m, QString path = QString());

private:
	enum Include {
	    IncludeNothing=0,
	    IncludeMetadata=1,
	    IncludeThumbnails=2
	};

    QString *m_path;
	QList<Metadata*> includes;
	int m_loadedIncludes;
    QSettings *m_settings;
	QString currentAppLang;
	QString lang;
	QStringList columnLabels;
	QString label;
	QHash<QString, Include> m_includeHash;
    QList<Item*> m_thumbItems;
	bool m_includedData;

	QString buildIncludePath(QString raw);
	QStringList buildIncludePaths(QStringList raw);
	void setIncludeMark(QStringList &list, Include mark);

signals:
	void includeRequired(Item *item, QString path);
	void includeRequireCancelled(Item *item);
	void ready(Item *item);
	void retranslated();
};
#endif
#endif // METADATA_H
