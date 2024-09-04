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
#include "filemover.h"
#include "partselector.h"
#include "partcache.h"
#include "prtreader.h"

FileModel::FileModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_iconProvider = new FileIconProvider();
    m_thumb = new ThumbnailManager(this);
    m_prtReader = new PrtReader(this);
    connect(m_thumb, SIGNAL(updateModel()), this, SLOT(updateThumbnails()));
    connect(PartCache::get(), SIGNAL(cleared(QString)),
            this, SLOT(directoryCleared(QString)));
    connect(PartCache::get(), SIGNAL(directoryRenamed(QString,QString)),
            this, SLOT(directoryRenamed(QString,QString)));
    connect(PartCache::get(), SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChanged(QString)));
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
    return PartCache::get()->count(m_path);
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    auto pc = PartCache::get();

    if (!index.isValid() || m_path.isEmpty() || !pc->count(m_path))
        return QVariant();

    QFileInfo part = pc->partAt(m_path, index.row());
    const int col = index.column();

    // first handle standard QFileSystemModel data
    if (col == 0 && role == Qt::CheckStateRole)
    {
        return PartSelector::get()->isSelected(
                   m_path,
                   part.absoluteFilePath()
               );
    }
    else if (col == 0 && role == Qt::DisplayRole)
    {
        return part.fileName();
    }
    else if (col == 0 && role == Qt::DecorationRole)
    {
        return m_iconProvider->icon(part);
    }
    // custom columns:
    // thumbnail
    else if (col == 1)
    {
        QString key(part.baseName());
        switch( role )
        {
        case Qt::DecorationRole:
        {
            // TODO/FIXME: this is quite slow. Think about optimization
            FileMetadata m(part);
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
                return m_thumb->thumbnail(part);
            break;
        }
        case Qt::SizeHintRole:
            return QSize(Settings::get()->GUIThumbWidth,
                         Settings::get()->GUIThumbWidth);
            break;
        case Qt::ToolTipRole:
            return m_thumb->tooltip(part);
            break;
        }
    } // additional metadata
    else if (role == Qt::DisplayRole && col > 1)
    {
        return MetadataCache::get()->partParam(
                   m_path,
                   part.fileName(),
                   m_parameterHandles[col - 2]
               );
    }

    return QVariant();
}

void FileModel::updateThumbnails()
{
    // TODO/FIXME: proper index subset for udpate
    beginResetModel();
    endResetModel();
}

QFileInfo FileModel::fileInfo(const QModelIndex &ix)
{
    return PartCache::get()->partAt(m_path, ix.row());
}

QFileInfoList FileModel::fileInfoList()
{
    return PartCache::get()->parts(m_path);
}

QString FileModel::findThumbnailPath(const QFileInfo &fi)
{
    return m_thumb->path(fi);
}

void FileModel::setupColumns(const QString &path)
{
    m_columnLabels.clear();
    m_columnLabels << tr("Part name") << "Thumbnail";
    m_columnLabels << MetadataCache::get()->parameterLabels(path);

    m_parameterHandles = MetadataCache::get()->parameterHandles(path);
}

void FileModel::directoryCleared(const QString &dir)
{
    if (dir == m_path)
        refreshModel();
}

void FileModel::directoryRenamed(const QString &oldName, const QString &newName)
{
    if (oldName == m_path)
        m_path = newName;
}

void FileModel::directoryChanged(const QString &dir)
{
    if (dir == m_path)
        refreshModel();
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
    int col = index.column();
    int flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;

    if (col > 1)
        flags |= Qt::ItemIsEditable;
    else if (col == 0)
        flags |= Qt::ItemIsDragEnabled;

    return (Qt::ItemFlag) flags;
}

QStringList FileModel::mimeTypes() const
{
    return QStringList() << "text/uri-list";
}

QMimeData *FileModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    auto pc = PartCache::get();
    auto selector = PartSelector::get();
    auto it = selector->allSelectedIterator();

    foreach (const QModelIndex &idx, indexes) {
        if (!idx.isValid())
            continue;

        QFileInfo part = pc->partAt(m_path, idx.row());
        urls << QUrl::fromLocalFile(part.absoluteFilePath());
    }

    while (it.hasNext()) {
        it.next();

        QStringList parts = it.value();

        foreach (const QString &part, parts) {
            QUrl url = QUrl::fromLocalFile(part);

            if (!urls.contains(url))
                urls << url;
        }
    }

    selector->clear();

    mimeData->setUrls(urls);
    return mimeData;
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QFileInfo part = fileInfo(index);

    if (role == Qt::CheckStateRole)
    {
        bool selected = PartSelector::get()->toggle(
            m_path,
            part.absoluteFilePath()
        );

        emit dataChanged(index, index);

        // Toggle other items selected in the view
        foreach (const QModelIndex &selectedIndex, m_selectedIndexes) {
            if (selectedIndex.row() == index.row())
                continue;

            QFileInfo selectedPart = fileInfo(selectedIndex);

            if (selected) {
                PartSelector::get()->select(
                    m_path,
                    selectedPart.absoluteFilePath()
                );
            } else {
                PartSelector::get()->clear(
                    m_path,
                    selectedPart.absoluteFilePath()
                );
            }

            emit dataChanged(selectedIndex, selectedIndex);
        }

        return true;

    } else if (role == Qt::EditRole && index.column() > 1) {
        MetadataCache::get()->metadata(m_path)->setPartParam(
            part.fileName(),
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

void FileModel::setSelectedIndexes(const QModelIndexList &list)
{
    m_selectedIndexes = list;
}

void FileModel::setDirectory(const QString &path)
{
    // this has to go before path != m_path check to reload the header translations
    setupColumns(path);

    if (path != m_path)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        m_thumb->setPath(path);
        m_prtReader->stop();
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

    beginResetModel();
    endResetModel();

    QApplication::restoreOverrideCursor();
}

void FileModel::reloadParts()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    PartCache::get()->refresh(m_path);

    setupColumns(m_path);
    m_thumb->clear();
    m_prtReader->load(m_path, fileInfoList());

    QApplication::restoreOverrideCursor();
}

void FileModel::reloadThumbnails()
{
    m_thumb->clear();
}

void FileModel::settingsChanged()
{
    //setDirectory(m_path);
}


void FileModel::moveParts(FileMover *mv)
{
    QStringList clearList;
    auto selector = PartSelector::get();
    auto pc = PartCache::get();
    auto it = selector->allSelectedIterator();
    auto dstDir = Settings::get()->getWorkingDir();
    auto metaCache = MetadataCache::get();

    while (it.hasNext())
    {
        it.next();

        QString dir = it.key();
        QStringList parts = it.value();

        foreach (const QString &fname, parts)
        {
            QFileInfo fi(fname);

            if (!Settings::get()->ShowProeVersions
                    && metaCache->partVersions(m_path).contains(fi.completeBaseName()))
            {
                // When moving Pro/E files, we need to find all part versions
                QDir d(m_path);
                mv->addSourceFiles(d.entryInfoList(QStringList() << (fi.completeBaseName() + ".*")));

            } else {
                mv->addSourceFile(fi);
            }

            QString thumbPath = m_thumb->path(fi);

            if (!thumbPath.isEmpty())
                mv->addSourceFile(QFileInfo(thumbPath), THUMBNAILS_DIR);

            if (fi.isDir())
                metaCache->clear(fname);

            metaCache->movePart(dir, fi.baseName(), dstDir);
        }

        clearList << dir;
    }

    mv->setDestination(dstDir);
    mv->work();

    selector->clear();

    foreach (const QString &dir, clearList)
        pc->clear(dir);

    pc->clear(dstDir);
}


void FileModel::deleteParts(DirectoryRemover *rm)
{
    // TODO: this code basically ignores all errors, so we delete metadata
    // of parts that are still on disk, e.g. because ZCP does not have permissions
    // to delete them. Undeleted parts are also unchecked. Errors are reported to
    // the user though via QMessageBox.

    QFileInfoList deleteList;
    QStringList clearList;
    auto selector = PartSelector::get();
    auto pc = PartCache::get();
    auto it = selector->allSelectedIterator();

    while (it.hasNext())
    {
        it.next();

        QString dir = it.key();
        QStringList parts = it.value();

        foreach (const QString &fname, parts)
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

            MetadataCache::get()->deletePart(dir, fi.baseName());
        }

        clearList << dir;
    }

    rm->addFiles(deleteList);
    rm->setStopOnError(false);
    rm->work();

    selector->clear();

    foreach (const QString &dir, clearList)
        pc->clear(dir);
}

void FileModel::copyToWorkingDir(FileCopier *cp)
{
    auto selector = PartSelector::get();
    auto it = selector->allSelectedIterator();
    auto dstDir = Settings::get()->getWorkingDir();
    auto metaCache = MetadataCache::get();

    while (it.hasNext())
    {
        it.next();
        QString key = it.key();

        // do not copy files from WD into WD
        if (key == dstDir)
        {
            selector->clear(key);
            continue;
        }

        foreach (const QString &fname, it.value())
        {
            QFileInfo fi(fname);
            cp->addSourceFile(fi);

            QString thumbPath = m_thumb->path(fi);

            if (!thumbPath.isEmpty())
                cp->addSourceFile(QFileInfo(thumbPath), THUMBNAILS_DIR);

            selector->clear(key, fname);

            metaCache->copyPart(key, fi.baseName(), dstDir);
        }
    }

    cp->setDestination(dstDir);
    cp->setStopOnError(false);

    cp->work();
    selector->clear();
    PartCache::get()->clear(dstDir);
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
