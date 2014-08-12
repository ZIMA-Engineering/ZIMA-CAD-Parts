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
#include <QMessageBox>

#include "filemodel.h"
#include "settings.h"


FileModel::FileModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    connect(MetadataCache::get(), SIGNAL(cleared()), this, SLOT(refreshModel()));
}

void FileModel::setDirectory(const QString &path)
{
#warning todo caching?
    qDeleteAll(m_data);
    m_data.clear();
    m_path = path;
    QDirIterator it(m_path, QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        it.next();
        m_data.append(new FileMetadata(it.fileInfo()));
    }

    m_columnLabels = MetadataCache::get()->columnLabels(m_path);

    beginResetModel();
    endResetModel();
}

void FileModel::refreshModel()
{
    setDirectory(m_path);
}

QFileInfo FileModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid())
        return QFileInfo();

    return m_data[index.row()]->fileInfo;
}

void FileModel::settingsChanged()
{
    beginResetModel();
    endResetModel();
}

int FileModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
    return 2 + m_columnLabels.count();
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_data.count();
    return 0;
}

QModelIndex FileModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();
}

QModelIndex FileModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (m_data.isEmpty())
		return QVariant();

	const int row = index.row();
	const int col = index.column();

    FileMetadata *file = m_data[row];

	switch(col)
	{
	case 1:
		switch( role )
		{
		case Qt::DecorationRole:
            if(file->thumbnail)
                return file->thumbnail->scaled(Settings::get()->GUIThumbWidth, Settings::get()->GUIThumbWidth);

		case Qt::SizeHintRole:
            return QSize(Settings::get()->GUIThumbWidth, file->thumbnail ? Settings::get()->GUIThumbWidth : 0);

		case Qt::ToolTipRole:
			if(file->thumbnail)
            {
                return QString("<img src=\"%1\" width=\"%2\">")
                        .arg(MetadataCache::get()->partThumbnailPath(m_path, file->fileInfo.absoluteFilePath()))
                        .arg(Settings::get()->GUIPreviewWidth);
            }
		}
		break;

	case 0:
		switch( role )
		{
		case Qt::DisplayRole:
			//qDebug() << "returning" << rootItem->files.at( index.row() )->name;
            return file->fileInfo.fileName();

		case Qt::CheckStateRole:
            return file->checked ? Qt::Checked : Qt::Unchecked;

		case Qt::DecorationRole: {
            QPixmap tmp = file->icon();
			if( !tmp.isNull() )
				return tmp.scaledToWidth(16);
		}
		}
		break;
	}

    if(col > 1 && role == Qt::DisplayRole && col-1 <= m_columnLabels.count())
	{
        return MetadataCache::get()->partParam(m_path, file->fileInfo.fileName(), col-1);
	}

	return QVariant();
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if( value.toString().isEmpty() )
		return false;

	if( role == Qt::CheckStateRole )
        m_data[ index.row() ]->checked = value.toBool();

	emit dataChanged(index, index);

	return true;
}

QVariant FileModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if( role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch( section )
	{
	case 1:
		return tr("Thumbnail");
	case 0:
		return tr("Part name");
	}

    if(section > 1 && section-2 < m_columnLabels.count())
	{
        return m_columnLabels[section - 2];
	}

	return QVariant();
}

bool FileModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(value);
	Q_UNUSED(role);
	return false;
}

Qt::ItemFlags FileModel::flags(const QModelIndex &index) const
{
	if( index.column() == 0 )
		return Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
	else return QAbstractItemModel::flags(index);
}

void FileModel::createIndexHtmlFile(const QString &text, const QString &fileBase)
{
    QDir techSpecDir(m_path + "/" + TECHSPEC_DIR);

    if(!techSpecDir.exists())
        techSpecDir.mkdir(techSpecDir.absolutePath());

    QString lang = Settings::get()->getCurrentLanguageCode().left(2);
    QFile indexFile(m_path + "/" + fileBase + "_" + lang + ".html");

    if(indexFile.exists())
    {
        if (QMessageBox::warning(0,
                                 tr("HTML index file already exists"),
                                 tr("HTML index for already exists, would you like to overwrite it?").arg(m_path),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }

    if(!indexFile.open(QIODevice::WriteOnly))
        return; // FIXME: Notify user on failure?

    QByteArray htmlIndex = QString("<html>\n"
                                   "	<head>\n"
                                   "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
                                   "	</head>\n"
                                   "</html>\n").arg(text).toUtf8();

    indexFile.write(htmlIndex);
    indexFile.close();
}

