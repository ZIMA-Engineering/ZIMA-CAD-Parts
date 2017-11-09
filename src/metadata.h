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

#include "file.h"

#define METADATA_VERSION 2

//! \brief Version map: completeBaseName -> fileName, only the latest version is stored in this map
typedef QHash<QString,QString> MetadataVersionsMap;

/*! Metadata for one directory.
 *
 * @warning do not access Metadata directly - use MetadataCache.
 *
 * Metadata are stored in 0000-index/metadata.ini as QSettings::IniFormat.
 * Groups:
 * [params]
 * <lang>/label = string // a directory label to be displayed in the directory tree
 * <lang>/1..n = string // a FileModel column label for additional columns
 *
 * [<base file name>] // name of the file without an extension
 * <lang>/1..n = string // a value which belongs to a column label
 *
 * Metadata Includes
 * [include]
 * data = path
 * thumbnails = path
 *
 */
class Metadata : public QObject
{
	Q_OBJECT
public:
	explicit Metadata(const QString &path, QObject *parent = 0);
	~Metadata();

	//! Label for current directory (tree)
	QString getLabel();
	QString getLabel(const QString &lang);

	void setLabel(const QString &lang, const QString &newLabel);

	bool showDirectoriesAsParts() const;
	void setShowDirectoriesAsParts(bool enabled);

	QStringList parameterHandles();
	void setParameterHandles(const QStringList &handles);

	//! Labels for FileModel
	QStringList parameterLabels();
	QStringList parameterLabels(const QString &lang);
	QHash<QString, QString> parametersWithLabels(const QString &lang);

	void setParameterLabel(const QString &param, const QString &lang, const QString &value);
	void renameParameter(const QString &handle, const QString &newHandle);
	void removeParameter(const QString &handle);

	//! Value for FileModel
	QString partParam(const QString &partName, const QString &param);
	QString partParam(const QString &partName, int index);

    //! Set new value for given param
	void setPartParam(const QString &partName, const QString &param, const QString &value);

	void deletePart(const QString &part);
	/*! Load part versions.
	 * Implementation: list name-ordered directory and use only the latest
	 * file name for its completeBaseName
	 */
	MetadataVersionsMap partVersions();

    QList<Metadata*> includes() { return m_includes; }
    QString path() { return m_path; }

    void reloadProe(const QFileInfoList &fil);

private:
	QSettings *m_settings;

	QString m_path;
    QList<Metadata*> m_includes;
	int m_loadedIncludes;
	QStringList m_parameterLabels;
	QString label;

	MetadataVersionsMap m_versionsCache;

	void setup();
	int version();
	bool isEmpty();
	QString buildIncludePath(const QString &raw);
	QStringList buildIncludePaths(const QStringList &raw);
	bool partVersionType(FileType::FileType t, const QFileInfo &fi);
	void rename(const QString &oldName, const QString &newName);
	void recursiveRename(const QString &path, QHash<QString, QVariant> &settings);
};

/*! An access singleton to the Metadata cache.
 * All-aware universal key is the "path" - the full path of the directory
 * used in the FileModel.
 */
class MetadataCache : public QObject
{
	Q_OBJECT
public:

	//! The main access method to Metadata
	static MetadataCache *get();

	bool showLabel(const QString &path);
	QString label(const QString &path);
	bool showDirectoriesAsParts(const QString &path);
	QStringList parameterHandles(const QString &path);
	QStringList parameterLabels(const QString &path);
	QString partParam(const QString &path, const QString &fname, const QString &param);
	QString partParam(const QString &path, const QString &fname, int index);
	MetadataVersionsMap partVersions(const QString &path);
	void deletePart(const QString &path, const QString &part);
    Metadata* metadata(const QString &path);

signals:
	//! Emitted when is the cache content invalidated. All dependent objects should reset themself.
	void cleared();

public slots:
	void clear();
	void clear(const QString &path);

private:
	//! Singleton handling
	static MetadataCache *m_instance;

	MetadataCache();
	//MetadataCache(const MetadataCache &) {};
	~MetadataCache();

	QHash<QString,Metadata*> m_map;
	void load(const QString &path);
};

#endif // METADATA_H
