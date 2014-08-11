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


class Metadata : public QObject
{
	Q_OBJECT
public:
    explicit Metadata(const QString &path, QObject *parent = 0);
	~Metadata();

	QString getLabel();
    QStringList columnLabels();
    QString partParam(const QString &partName, int col);
    QString partThumbnailPath(const QString &partName);
    void deletePart(const QString &part);

public slots:
	void retranslate(QString lang = QString());

private:
	enum Include {
	    IncludeNothing=0,
	    IncludeMetadata=1,
	    IncludeThumbnails=2
	};

    QSettings *m_settings;

    QString m_path;
	QList<Metadata*> includes;
	int m_loadedIncludes;
    QString m_currentAppLang;
	QString lang;
    QStringList m_columnLabels;
	QString label;
	bool m_includedData;

    QString buildIncludePath(const QString &raw);
    QStringList buildIncludePaths(const QStringList &raw);
};


class MetadataCache
{
public:

    //! The main access method to Metadata
    static MetadataCache *get();

    void clear();
    QStringList columnLabels(const QString &path);
    QString partParam(const QString &path, const QString &fname, int column);
    QPixmap* partThumbnail(const QString &path, const QString fname);

private:
    //! Singleton handling
    static MetadataCache *m_instance;

    MetadataCache();
    MetadataCache(const MetadataCache &) {};
    ~MetadataCache();

    QHash<QString,Metadata*> m_map;
    void load(const QString &path);
};

#endif // METADATA_H
