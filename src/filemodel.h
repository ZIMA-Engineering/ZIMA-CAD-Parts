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

#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QSortFilterProxyModel>

#include "item.h"

class FileItem
{
public:
    FileItem(const QFileInfo &fi)
    {
        file = new File(fi);
        checked = false;
        thumbnail = MetadataCache::get()->partThumbnail(fi.path(), fi.fileName());
    }
    ~FileItem() {
        delete(file);
        delete(thumbnail);
    }

    File *file;
    bool checked;
    QPixmap *thumbnail;
};

class FileModel : public QAbstractItemModel
{
	Q_OBJECT
public:
    explicit FileModel(QObject *parent = 0);

	//---
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    //QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    //QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	//---

//	Item* getRootItem();
//	void setPreviewWidth(int size);

    QFileInfo fileInfo(const QModelIndex &index) const;
    void createIndexHtmlFile(const QString &text, const QString &fileBase);

public slots:
//	void setRootIndex(const QModelIndex &index);
//	void setRootIndex(Item *item);
    void setDirectory(const QString &path);
#warning    void initMetadata(Item *i);
    void settingsChanged();

private:
    QString m_path;
    QStringList m_columnLabels;
    QList<FileItem*> m_data;

private slots:
    void thumbnailDownloaded(File *file);
//	void itemLoaded(Item *item);
    void metadataRetranslated();

signals:
	void requestColumnResize();
};

#endif // FILEMODEL_H
