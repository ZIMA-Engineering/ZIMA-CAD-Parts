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

#ifndef SERVERSMODEL_H
#define SERVERSMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QKeyEvent>

#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QFileIconProvider>


class ServersModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit ServersModel(QObject *parent = 0);

    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
};

class ServersIconProvider : public QFileIconProvider
{
public:
    ServersIconProvider();

    virtual QIcon   icon ( IconType type ) const;
    virtual QIcon   icon ( const QFileInfo & info ) const;
    virtual QString type ( const QFileInfo & info ) const;
};


class ServersProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ServersProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const;
};

#endif // SERVERSMODEL_H
