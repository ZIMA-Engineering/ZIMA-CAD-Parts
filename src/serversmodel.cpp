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

#include "serversmodel.h"
#include <QDir>
#include <QDebug>
#include "settings.h"

ServersModel::ServersModel(QObject *parent) :
    QFileSystemModel(parent)
{
    setReadOnly(true);
    setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    setIconProvider(new ServersIconProvider());
}

int ServersModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ServersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
        if (MetadataCache::get()->label(filePath(index)).isEmpty())
            return QFileSystemModel::data(index, role);
        else
            return MetadataCache::get()->label(filePath(index));
    case Qt::ToolTipRole:
        return fileInfo(index).absoluteFilePath();
    default:
        ;
    }

    return QFileSystemModel::data(index, role);
}


ServersProxyModel::ServersProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}


bool ServersProxyModel::filterAcceptsRow(int sourceRow,
                                            const QModelIndex &sourceParent) const
{
    QModelIndex ix = sourceModel()->index(sourceRow, 0, sourceParent);
    return ix.data().toString() != TECHSPEC_DIR;
}

ServersIconProvider::ServersIconProvider()
{
}

QIcon ServersIconProvider::icon ( IconType type ) const
{
    return QFileIconProvider::icon(type);
}

QIcon ServersIconProvider::icon ( const QFileInfo & info ) const
{
    QString logoPath = info.absoluteFilePath() +"/"+ TECHSPEC_DIR +"/";

    if (!info.isDir())
    {
        return QFileIconProvider::icon(info);
    }
    else if (QFile::exists(logoPath + LOGO_FILE))
    {
        return QIcon(logoPath + LOGO_FILE);
    }
    else if (QFile::exists(logoPath + LOGO_TEXT_FILE))
    {
        return QIcon(logoPath + LOGO_TEXT_FILE);
    }
    else
        return QFileIconProvider::icon(info);
}

QString ServersIconProvider::type ( const QFileInfo & info ) const
{
    return QFileIconProvider::type(info);
}
