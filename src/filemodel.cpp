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
                return QPixmap(m.fileInfo.absoluteFilePath()).scaled(Settings::get()->GUIThumbWidth,
                                                                     Settings::get()->GUIThumbWidth,
                                                                     Qt::KeepAspectRatio);
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
		return MetadataCache::get()->partParam(m_path,
		                                       m_data.at(index.row()).fileName(),
		                                       col-1);
	}

	return QVariant();
}

void FileModel::loadFiles(const QString &path)
{
	m_data.clear();
	QDir d(path);
	m_data = d.entryInfoList(QDir::Files|QDir::Readable, QDir::Name);
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

	} else if (role == Qt::EditRole) {
		MetadataCache::get()->metadata(m_path)->setPartParam(
			m_data.at(index.row()).fileName(),
			index.column() - 1,
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
	m_columnLabels = MetadataCache::get()->columnLabels(path);

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

    m_columnLabels = MetadataCache::get()->columnLabels(m_path);
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

void FileModel::deleteParts()
{
	QFile f;
	QHash<QString,QString> errors;
	QFileInfo key;

	foreach (QString fname, m_checked[m_path])
	{
		key.setFile(fname);

		if (!Settings::get()->ShowProeVersions
		        && MetadataCache::get()->partVersions(m_path).contains(key.completeBaseName()))
		{
			QDir d(m_path);
			QFileInfoList fil = d.entryInfoList(QStringList() << key.completeBaseName()+".*");
			foreach (QFileInfo fi, fil)
			{
				if (!f.remove(fi.absoluteFilePath()))
				{
					errors[fi.absoluteFilePath()] = f.errorString();
				}
				else
				{
					m_data.removeAll(key);
					MetadataCache::get()->deletePart(m_path, key.baseName());
					m_checked[m_path].removeAll(fname);
				}
			}
		}
		else
		{
			if (!f.remove(key.absoluteFilePath()))
				errors[key.absoluteFilePath()] = f.errorString();
			else
			{
				m_data.removeAll(key);
				MetadataCache::get()->deletePart(m_path, key.baseName());
				m_checked[m_path].removeAll(fname);
			}
		}
	}

	if (errors.count())
	{
		ErrorDialog dlg;
		dlg.setErrors(tr("File deletion error(s):"), errors);
		dlg.exec();
	}
	else
	{
		m_checked[m_path].clear();
		beginResetModel();
		endResetModel();
	}
}

void FileModel::copyToWorkingDir()
{
	bool overwrite = false;
	QHash<QString,QString> errors;
	QDir d;

    if (!d.exists(Settings::get()->getWorkingDir()))
	{
		if (QMessageBox::question(0, tr("Working Dir does not exist"),
                                  tr("Working directory %1 does not exist. Create it?").arg(Settings::get()->getWorkingDir()),
		                          QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
        if (!d.mkpath(Settings::get()->getWorkingDir()))
            errors[Settings::get()->getWorkingDir()] = "Cannot create directory: " + Settings::get()->getWorkingDir();
	}

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

		foreach (QString fname, it.value())
		{
			QFileInfo fi(fname);
			QFile f(fi.absoluteFilePath());
            QString target = Settings::get()->getWorkingDir() + "/" + fi.fileName();

			if (!overwrite && QDir().exists(target))
			{
				QMessageBox::StandardButton ret = QMessageBox::question(0, tr("Overwrite file?"),
				                                  tr("File %1 already exists. Overwrite?").arg(target),
				                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::YesAll|QMessageBox::Cancel
				                                                       );
				switch (ret)
				{
				case QMessageBox::Yes:
					break;
				case QMessageBox::No:
					continue;
					break;
				case QMessageBox::YesAll:
					overwrite = true;
					break;
				case QMessageBox::Cancel:
					return;
					break;
				default:
					break;
				} // switch

			} // if !overwrite

			if (f.exists(target) && !f.remove(target))
			{
				errors[target] = f.errorString();
				continue;
			}
			if (!f.copy(target))
				errors[target] = f.errorString();
			else
			{
				// copy the thumbnail too
                if (QDir().mkpath(Settings::get()->getWorkingDir() + "/" + THUMBNAILS_DIR))
				{
                    QString fp = m_thumb->path(fi.baseName());
                    if (fp.isEmpty())
                        continue;
                    QFile thumb(fp);
                    thumb.copy(Settings::get()->getWorkingDir() + "/" + THUMBNAILS_DIR +"/" + fi.baseName()+".jpg");

				}
				m_checked[key].removeAll(fname);
			}
		} // while

	}

	if (errors.count())
	{
		ErrorDialog dlg;
		dlg.setErrors(tr("File copying error(s):"), errors);
		dlg.exec();
	}
	else
	{
		m_checked.clear();
		beginResetModel();
		endResetModel();
	}
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
