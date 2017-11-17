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

#ifndef DATASOURCEMODEL_H
#define DATASOURCEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QKeyEvent>

#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QFileIconProvider>

class DataSourceIconProvider;


/*! A "datasource" tree. This class is used inside ServersView only
 */
class DataSourceModel : public QFileSystemModel
{
	Q_OBJECT
public:
	explicit DataSourceModel(QObject *parent = 0);

	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;

private:
	DataSourceIconProvider *m_iconProvider;
};

/*! \brief An icon provider for ServersModel. It contains additional
 * handling of LOGO_FILE and LOGO_FILE_TEXT files in the TECHSPEC_DIR directory
 * The main method to be used is custom pixmap().
 */
class DataSourceIconProvider : public QFileIconProvider
{
public:
	DataSourceIconProvider();

	virtual QIcon   icon ( IconType type ) const;
	virtual QIcon   icon ( const QFileInfo & info ) const;
	virtual QString type ( const QFileInfo & info ) const;

	/*! \brief Handle custom logo files or standard file browser icons as QPixmaps.
	 * The request is simple - to allow various sized pixmaps to be displayed.
	 * Basically there is no image resizing allowed. Logo files are used as-is,
	 * standard icons are resized to 32x32
	 */
	QPixmap pixmap(const QFileInfo &info) const;
};

/*! A filter proxy for ServersModel. Its main task is to filter out
 * the TECHSPEC_DIR directory
 */
class DataSourceProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	DataSourceProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int sourceRow,
	                      const QModelIndex &sourceParent) const;
};

#endif // DATASOURCEMODEL_H
