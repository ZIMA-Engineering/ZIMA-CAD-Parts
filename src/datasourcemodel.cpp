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

#include "datasourcemodel.h"
#include <QDir>
#include <QDebug>
#include "settings.h"

DataSourceModel::DataSourceModel(QObject *parent) :
	QFileSystemModel(parent)
{
	setReadOnly(true);
	setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
	m_iconProvider = new DataSourceIconProvider();
	// do not install icon provider. The icon handling is quite complex. See ServersIconProvider
	//setIconProvider(m_iconProvider);
}

int DataSourceModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
	return 1;
}

QVariant DataSourceModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	switch(role)
	{
	case Qt::DisplayRole: {
		if (!MetadataCache::get()->showLabel(filePath(index)))
			return QVariant();

		QString label = MetadataCache::get()->label(filePath(index));

		if (label.isEmpty())
			return QFileSystemModel::data(index, role);

		else
			return MetadataCache::get()->label(filePath(index));
	}

	case Qt::ToolTipRole:
		return fileInfo(index).absoluteFilePath();

	case Qt::DecorationRole:
		return m_iconProvider->pixmap(fileInfo(index));

	default:
		break;
	}

	return QFileSystemModel::data(index, role);
}


DataSourceProxyModel::DataSourceProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{
}


bool DataSourceProxyModel::filterAcceptsRow(int sourceRow,
        const QModelIndex &sourceParent) const
{
	QModelIndex ix = sourceModel()->index(sourceRow, 0, sourceParent);
	return ix.data().toString() != METADATA_DIR;
}

DataSourceIconProvider::DataSourceIconProvider()
{
}

QIcon DataSourceIconProvider::icon ( IconType type ) const
{
	return QFileIconProvider::icon(type);
}

QIcon DataSourceIconProvider::icon ( const QFileInfo & info ) const
{
	QString logoPath = info.absoluteFilePath() +"/"+ METADATA_DIR +"/";

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

QPixmap DataSourceIconProvider::pixmap(const QFileInfo &info) const
{
	QString logoPath = info.absoluteFilePath() +"/"+ METADATA_DIR +"/";
	if (QFile::exists(logoPath + LOGO_FILE))
	{
		return QPixmap(logoPath + LOGO_FILE);
	}
	else if (QFile::exists(logoPath + LOGO_TEXT_FILE))
	{
		return QPixmap(logoPath + LOGO_TEXT_FILE);
	}
	else
	{
		return QFileIconProvider::icon(info).pixmap(32, 32);
	}
}

QString DataSourceIconProvider::type ( const QFileInfo & info ) const
{
	return QFileIconProvider::type(info);
}
