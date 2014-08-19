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

#include <QDebug>

#include "filemodel.h"
#include "settings.h"

FileModel::FileModel(QObject *parent) :
    QFileSystemModel(parent)
{
    setReadOnly(true);
    setFilter(QDir::Files | QDir::NoDotAndDotDot);
    setIconProvider(new FileIconProvider());

    connect(this, SIGNAL(directoryLoaded(QString)),
            this, SLOT(loadThumbnails(QString)));
}

int FileModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return QFileSystemModel::columnCount() + m_columnLabels.count();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int col = index.column();

    // first handle standard QFileSystemModel data
    if (col == 0 && role == Qt::CheckStateRole)
    {
        return m_checked[index];
    }
    else if (col < QFileSystemModel::columnCount())
    {
        return QFileSystemModel::data(index, role);
    }
    // custom columns:
    // thumbnail
    else if (col == QFileSystemModel::columnCount())
    {
        QString key(fileInfo(index).baseName());
        switch( role )
        {
        case Qt::DecorationRole:
            if (m_thumbnails.contains(key))
                return m_thumbnails[key];
            break;
        case Qt::SizeHintRole:
            return QSize(Settings::get()->GUIThumbWidth, m_thumbnails.contains(key) ? Settings::get()->GUIThumbWidth : 0);
            break;
        case Qt::ToolTipRole:
            if (m_thumbnails.contains(key))
            {
                return QString("<img src=\"%1\" width=\"%2\">")
                        //.arg(MetadataCache::get()->partThumbnailPaths(m_path, fileInfo(index).absoluteFilePath()))
                        .arg(m_thumbnailPath[key])
                        .arg(Settings::get()->GUIPreviewWidth);
            }
            break;
        }
    } // additional metadata
    else if (role == Qt::DisplayRole && col > QFileSystemModel::columnCount())
    {
        return MetadataCache::get()->partParam(m_path,
                                               fileInfo(index).fileName(),
                                               col-QFileSystemModel::columnCount()-1);
    }

    return QVariant();
}

void FileModel::loadThumbnails(const QString &path)
{
    QHashIterator<QString,QString> it(MetadataCache::get()->partThumbnailPaths(m_path));
    while (it.hasNext())
    {
        it.next();
        QPixmap pm(it.value());
        if (!pm.isNull())
        {
            m_thumbnails[it.key()] = pm.scaled(Settings::get()->GUIThumbWidth, Settings::get()->GUIThumbWidth);
            m_thumbnailPath[it.key()] = it.value();
        }
    }

    emit dataChanged(index(0, QFileSystemModel::columnCount()),
                     index(rowCount()-1, QFileSystemModel::columnCount()));
    //beginResetModel();
    //endResetModel();
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        m_checked[index] = value.toBool() ? Qt::Checked : Qt::Unchecked;
        emit dataChanged(index, index);
        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

QVariant FileModel::headerData (int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    if (section == 0)
        return tr("Part name");
    else if (section < QFileSystemModel::columnCount())
        return QFileSystemModel::headerData(section, orientation, role);
    else if (section == QFileSystemModel::columnCount())
        return tr("Thumbnail");
    else if (section > QFileSystemModel::columnCount())
    {
        return m_columnLabels[section - QFileSystemModel::columnCount()-1];
    }

    return QVariant();
}

void FileModel::setDirectory(const QString &path)
{
    m_thumbnailPath.clear();
    m_thumbnails.clear();
    m_checked.clear();
    m_path = path;
    m_columnLabels = MetadataCache::get()->columnLabels(m_path);
}

void FileModel::deleteParts()
{
    QHashIterator<QModelIndex,Qt::CheckState> it(m_checked);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == Qt::Unchecked)
            continue;
        if (Settings::get()->ShowProeVersions)
        {
            // delete all "versioned" files
        }
        else
            qDebug() << remove(it.key());
    }
    m_checked.clear();
}


FileIconProvider::FileIconProvider()
{
}

QIcon FileIconProvider::icon ( IconType type ) const
{
    return QFileIconProvider::icon(type);
}

QIcon FileIconProvider::icon ( const QFileInfo & info ) const
{
#warning todo get rid of FileMetadata. getInternalNameForFileType(QFileInfo) should be enough
    FileMetadata fi(info);
    QString s = QString(":/gfx/icons/%1.png").arg(File::getInternalNameForFileType(fi.type));

    if (QFile::exists(s))
        return QPixmap(s);

    return QFileIconProvider().icon(info).pixmap(64);
}

QString FileIconProvider::type ( const QFileInfo & info ) const
{
    return QFileIconProvider::type(info);
}
