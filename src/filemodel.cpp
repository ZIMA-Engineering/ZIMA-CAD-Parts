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
#include <QApplication>

#include "filemodel.h"
#include "settings.h"
#include "errordialog.h"
#include "directoryremover.h"
#include "filecopier.h"

FileModel::FileModel(QObject *parent) :
	QAbstractItemModel(parent)
{
	m_iconProvider = new FileIconProvider();
    m_thumb = new ThumbnailManager(this);
    connect(m_thumb, SIGNAL(updateModel()), this, SLOT(updateThumbnails()));
}

FileModel::~FileModel()
{
	delete m_iconProvider;
}

QModelIndex FileModel::index(int row, int column,
                             const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

QModelIndex FileModel::parent(const QModelIndex &child) const
{
	Q_UNUSED(child);
	return QModelIndex();
}

int FileModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
	return m_columnLabels.count();
}

int FileModel::rowCount(const QModelIndex & parent) const
{
	if (!parent.column()) return 0;
	return m_data.size();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || m_path.isEmpty() || !m_data.size())
		return QVariant();

	const int col = index.column();

	// first handle standard QFileSystemModel data
	if (col == 0 && role == Qt::CheckStateRole)
	{
        return QVariant(m_checked[m_path].contains(m_data.at(index.row()).absoluteFilePath()));
	}
	else if (col == 0 && role == Qt::DisplayRole)
	{
		return m_data.at(index.row()).fileName();
	}
	else if (col == 0 && role == Qt::DecorationRole)
	{
		return m_iconProvider->icon(m_data.at(index.row()));
	}
	// custom columns:
	// thumbnail
	else if (col == 1)
	{
		QString key(m_data.at(index.row()).baseName());
		switch( role )
		{
		case Qt::DecorationRole:
        {
            // TODO/FIXME: this is quite slow. Think about optimization
            FileMetadata m(m_data.at(index.row()));
            // generate thumbnail for image files
            if (m.type == FileType::FILE_IMAGE)
            {
				return QPixmap(m.fileInfo.absoluteFilePath()).scaled(
					Settings::get()->GUIThumbWidth,
					Settings::get()->GUIThumbWidth,
					Qt::KeepAspectRatio
				);
            }
            else
                return m_thumb->thumbnail(m_data.at(index.row()));
			break;
        }
		case Qt::SizeHintRole:
            return QSize(Settings::get()->GUIThumbWidth,
                         Settings::get()->GUIThumbWidth);
            break;
		case Qt::ToolTipRole:
            return m_thumb->tooltip(m_data.at(index.row()));
			break;
		}
	} // additional metadata
	else if (role == Qt::DisplayRole && col > 1)
	{
		return MetadataCache::get()->partParam(
			m_path,
			m_data.at(index.row()).fileName(),
			m_parameterHandles[col - 2]
		);
	}

	return QVariant();
}

void FileModel::loadFiles(const QString &path)
{
	m_data.clear();
	QDir d(path);
	m_data = d.entryInfoList(
		QDir::Files
		| QDir::Dirs
		| QDir::Readable
		| QDir::NoDotAndDotDot,
		QDir::Name
	);
}

void FileModel::updateThumbnails()
{
    // TODO/FIXME: proper index subset for udpate
    beginResetModel();
    endResetModel();
}

QFileInfo FileModel::fileInfo(const QModelIndex &ix)
{
	return m_data.at(ix.row());
}

void FileModel::setupColumns(const QString &path)
{
	m_columnLabels.clear();
	m_columnLabels << tr("Part name") << "Thumbnail";
	m_columnLabels << MetadataCache::get()->parameterLabels(path);

	m_parameterHandles = MetadataCache::get()->parameterHandles(path);
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
	int col = index.column();
	int flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;

	if (col > 1)
		flags |= Qt::ItemIsEditable;

	return (Qt::ItemFlag) flags;
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::CheckStateRole)
	{
		QString path = m_data.at(index.row()).absoluteFilePath();

		if (m_checked[m_path].contains(path))
			m_checked[m_path].removeAll(path);
		else
			m_checked[m_path].append(path);

		emit dataChanged(index, index);
		return true;

	} else if (role == Qt::EditRole && index.column() > 1) {
		MetadataCache::get()->metadata(m_path)->setPartParam(
			m_data.at(index.row()).fileName(),
			m_parameterHandles[ index.column() - 2 ],
			value.toString()
		);

		emit dataChanged(index, index);
		return true;
	}

	return QAbstractItemModel::setData(index, value, role);
}

QVariant FileModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if( role != Qt::DisplayRole || orientation != Qt::Horizontal || !m_columnLabels.size())
		return QVariant();

	return m_columnLabels[section];
}

void FileModel::setDirectory(const QString &path)
{
	// this has to go before path != m_path check to reload the header translations
	setupColumns(path);

	if (path != m_path)
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		loadFiles(path);
        m_thumb->setPath(path);
		m_path = path;

		beginResetModel();
		endResetModel();

		QApplication::restoreOverrideCursor();
	}

	// simulating "directory loaded" signal even when is the path the
	// same as before to recalculate the column sizes in view
	emit directoryLoaded(m_path);
}

void FileModel::refreshModel()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	setupColumns(m_path);

    loadFiles(m_path);
    m_thumb->clear();

    beginResetModel();
    endResetModel();

    QApplication::restoreOverrideCursor();
}

void FileModel::settingsChanged()
{
	//setDirectory(m_path);
}

void FileModel::deleteParts(DirectoryRemover *rm)
{
	// TODO: this code basically ignores all errors, so we delete metadata
	// of parts that are still on disk, e.g. because ZCP does not have permissions
	// to delete them. Undeleted parts are also unchecked. Errors are reported to
	// the user though via QMessageBox.

	QFileInfoList deleteList;

	beginResetModel();
	m_data.clear();

	foreach (const QString &fname, m_checked[m_path])
	{
		QFileInfo fi(fname);

		if (!Settings::get()->ShowProeVersions
				&& MetadataCache::get()->partVersions(m_path).contains(fi.completeBaseName()))
		{
			// When deleting Pro/E files, we need to find all part versions
			QDir d(m_path);
			deleteList << d.entryInfoList(
				QStringList() << (fi.completeBaseName() + ".*")
			);

		} else {
			deleteList << fi;
		}

		if (fi.isDir())
			MetadataCache::get()->clear(fname);

		m_data.removeAll(fi);
		MetadataCache::get()->deletePart(m_path, fi.baseName());
		m_checked[m_path].removeAll(fname);
	}

	rm->addFiles(deleteList);
	rm->setStopOnError(false);
	rm->work();

	m_checked[m_path].clear();
	loadFiles(m_path);

	endResetModel();
}

void FileModel::copyToWorkingDir(FileCopier *cp)
{
	QHashIterator<QString,QStringList> it(m_checked);

	while (it.hasNext())
	{
		it.next();
		QString key = it.key();

		// do not copy files from WD into WD
		if (key == Settings::get()->getWorkingDir())
		{
			m_checked[key].clear();
			continue;
		}

		foreach (const QString &fname, it.value())
		{
			QFileInfo fi(fname);
			cp->addSourceFile(fi);

			QString thumbPath = m_thumb->path(fi.baseName());

			if (!thumbPath.isEmpty())
				cp->addSourceFile(QFileInfo(thumbPath), THUMBNAILS_DIR);

			m_checked[key].removeAll(fname);
		}
	}

	cp->setDestination(Settings::get()->getWorkingDir());
	cp->setStopOnError(false);

	beginResetModel();

	cp->work();
	m_checked.clear();

	endResetModel();
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
