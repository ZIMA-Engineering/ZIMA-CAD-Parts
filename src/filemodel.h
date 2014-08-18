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
#include <QPixmapCache>


/*! A "list files" tree. This class is used inside FileView only
 */
class FileModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit FileModel(QObject *parent = 0);

    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setDirectory(const QString &path);

    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QString m_path;
    QStringList m_columnLabels;
    QHash<QString,bool> m_checked;
    QHash<QString,QPixmap> m_thumbnails;
    QHash<QString,QString> m_thumbnailPath;

private slots:
    void loadThumbnails(const QString &path);
};

/*! An icon provider for FileModel. It contains additional
 * handling of CAD file icons from builtin resources
 */
class FileIconProvider : public QFileIconProvider
{
public:
    FileIconProvider();

    virtual QIcon   icon ( IconType type ) const;
    virtual QIcon   icon ( const QFileInfo & info ) const;
    virtual QString type ( const QFileInfo & info ) const;
};

#endif // FILEMODEL_H
