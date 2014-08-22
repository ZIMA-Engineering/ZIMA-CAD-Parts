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
        return m_checked[index];
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
            if (MetadataCache::get()->partThumbnailPaths(m_path).contains(key))
                return MetadataCache::get()->partThumbnailPaths(m_path)[key].second.scaled(Settings::get()->GUIThumbWidth,
                                                                                           Settings::get()->GUIThumbWidth);
            break;
        case Qt::SizeHintRole:
            if (MetadataCache::get()->partThumbnailPaths(m_path).contains(key))
            {
                return QSize(Settings::get()->GUIThumbWidth,
                             Settings::get()->GUIThumbWidth);
            }
            break;
        case Qt::ToolTipRole:
            if (MetadataCache::get()->partThumbnailPaths(m_path).contains(key))
            {
                return QString("<img src=\"%1\" width=\"%2\">")
                        .arg(MetadataCache::get()->partThumbnailPaths(m_path)[key].first)
                        .arg(Settings::get()->GUIPreviewWidth);
            }
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

QFileInfo FileModel::fileInfo(const QModelIndex &ix)
{
    return m_data.at(ix.row());
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        m_checked[index] = value.toBool() ? Qt::Checked : Qt::Unchecked;
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
        m_checked.clear();

        loadFiles(path);
        MetadataCache::get()->partThumbnailPaths(path);
        m_path = path;

        beginResetModel();
        endResetModel();

        QApplication::restoreOverrideCursor();
    }

    // simulating "directory loaded" signal even when is the path the
    // same as before to recalculate the column sizes in view
    emit directoryLoaded(m_path);
}

void FileModel::settingsChanged()
{
    //setDirectory(m_path);
}

void FileModel::deleteParts()
{
    QHashIterator<QModelIndex,Qt::CheckState> it(m_checked);
    QFile f;
    QHash<QString,QString> errors;
    QFileInfo key;

    while (it.hasNext())
    {
        it.next();
        if (it.value() == Qt::Unchecked)
            continue;
        key = m_data.at(it.key().row());

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
                    m_checked[it.key()] = Qt::Unchecked;
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
                m_checked[it.key()] = Qt::Unchecked;
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
        m_checked.clear();
}

void FileModel::copyToWorkingDir()
{
    QHashIterator<QModelIndex,Qt::CheckState> it(m_checked);
    bool overwrite = false;
    QHash<QString,QString> errors;
    QDir d;

    if (!d.exists(Settings::get()->WorkingDir))
    {
        if (QMessageBox::question(0, tr("Working Dir does not exist"),
                                  tr("Working directory %1 does not exist. Create it?").arg(Settings::get()->WorkingDir),
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
        if (!d.mkpath(Settings::get()->WorkingDir))
            errors[Settings::get()->WorkingDir] = "Cannot create directory: " + Settings::get()->WorkingDir;
    }

    while (it.hasNext())
    {
        it.next();
        if (it.value() != Qt::Checked)
            continue;

        QFileInfo fi = fileInfo(it.key());
        QFile f(fi.absoluteFilePath());
        QString target = Settings::get()->WorkingDir + "/" + fi.fileName();

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
            if (QDir().mkpath(Settings::get()->WorkingDir + "/" + THUMBNAILS_DIR))
            {
                QFileInfo thumbFi;
                thumbFi.setFile(MetadataCache::get()->partThumbnailPaths(m_path)[fi.baseName()].first);
                QFile thumb(thumbFi.absoluteFilePath());
                thumb.copy(Settings::get()->WorkingDir + "/" + THUMBNAILS_DIR +"/" + thumbFi.fileName());
            }
            m_checked[it.key()] = Qt::Unchecked;
        }
    } // while

    if (errors.count())
    {
        ErrorDialog dlg;
        dlg.setErrors(tr("File copying error(s):"), errors);
        dlg.exec();
    }
    else
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
