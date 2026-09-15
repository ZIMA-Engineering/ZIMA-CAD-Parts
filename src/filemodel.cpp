#include "directoryprotection.h"
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
#include "localfilters.h"
#include "errordialog.h"
#include "directoryremover.h"
#include "filecopier.h"
#include "filemover.h"
#include "partselector.h"
#include "partcache.h"
#include "prtreader.h"

namespace {
QFileInfoList versionFamily(const QFileInfo &selected)
{
    QFileInfoList result;
    const auto files = PartCache::get()->parts(selected.absolutePath());
    for (const QFileInfo &file : files) {
        if (!file.isDir() && file.completeBaseName() == selected.completeBaseName()
                && File::versionedTypes().contains(FileMetadata(file).type))
            result.append(file);
    }
    return result;
}
}

FileModel::FileModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_iconProvider = new FileIconProvider();
    m_thumb = new ThumbnailManager(this);
    m_prtReader = new PrtReader(this);
    connect(m_prtReader, &PrtReader::loaded, this, [this](const QFileInfo &part) {
        updatePartParameters(File::partBaseName(part));
    });
    connect(m_thumb, &ThumbnailManager::thumbnailReady, this, [this](const QString &file) {
        const auto row = m_thumbnailRows.constFind(file);
        if (row != m_thumbnailRows.cend())
            emit dataChanged(index(row.value(), 1), index(row.value(), 1),
                             {Qt::DecorationRole, Qt::ToolTipRole});
    });
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
    if (parent.isValid() || row < 0 || column < 0
            || row >= rowCount() || column >= columnCount())
        return QModelIndex();
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
    if (parent.isValid() || m_path.isEmpty()) return 0;
    return PartCache::get()->count(m_path);
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    auto pc = PartCache::get();

    if (!index.isValid() || m_path.isEmpty() || index.row() < 0
            || index.row() >= pc->count(m_path) || index.column() < 0
            || index.column() >= m_columnLabels.size())
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
        QString key(File::partBaseName(part));
        switch( role )
        {
        case Qt::DecorationRole:
        {
            return m_thumb->thumbnail(part);
            break;
        }
        case Qt::SizeHintRole:
            return QSize(Settings::get()->GUIThumbWidth,
                         Settings::get()->GUIThumbWidth);
            break;
        case Qt::ToolTipRole:
            return m_thumb->tooltip(part, Settings::get()->GUIPreviewWidth);
            break;
        }
    } // additional metadata
    else if (role == Qt::DisplayRole && col > 1 && col - 2 < m_parameterHandles.size())
    {
        return MetadataCache::get()->partParam(
                   m_path,
                   File::partBaseName(part),
                   m_parameterHandles[col - 2]
               );
    }

    return QVariant();
}

void FileModel::updatePartParameters(const QString &partName)
{
    if (columnCount() <= 2)
        return;
    const auto files = fileInfoList();
    for (int row = 0; row < files.size(); ++row) {
        if (File::partBaseName(files.at(row)) == partName)
            emit dataChanged(index(row, 2), index(row, columnCount() - 1),
                             {Qt::DisplayRole, Qt::EditRole});
    }
}

void FileModel::updateThumbnails()
{
    if (rowCount() > 0)
        emit dataChanged(index(0, 1), index(rowCount() - 1, 1),
                         {Qt::DecorationRole, Qt::ToolTipRole});
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
    m_columnLabels << tr("Part name") << tr("Thumbnail");
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
    if (oldName == m_path) {
        m_path = newName;
        refreshModel();
    }
}

void FileModel::directoryChanged(const QString &dir)
{
    if (dir == m_path)
        refreshModel();
}

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
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
            File::partBaseName(part),
            m_parameterHandles[ index.column() - 2 ],
            value.toString()
        );

        updatePartParameters(File::partBaseName(part));
        return true;
    }

    return QAbstractItemModel::setData(index, value, role);
}

QVariant FileModel::headerData (int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal
            || section < 0 || section >= m_columnLabels.size())
        return QVariant();

    return m_columnLabels[section];
}

void FileModel::setSelectedIndexes(const QModelIndexList &list)
{
    m_selectedIndexes = list;
}

void FileModel::setDirectory(const QString &path)
{
    if (path != m_path) {
        beginResetModel();
        m_prtReader->stop();
        m_path = path;
        setupColumns(path);
        m_thumb->setPath(path, Settings::get()->GUIThumbWidth);
        prepareThumbnailRows();
        endResetModel();
    }
    emit directoryLoaded(m_path);
}

void FileModel::prepareThumbnailRows()
{
    m_thumbnailRows.clear();
    const auto files = fileInfoList();
    for (int row = 0; row < files.size(); ++row)
        m_thumbnailRows.insert(files.at(row).absoluteFilePath(), row);
}

void FileModel::refreshModel()
{
    beginResetModel();
    setupColumns(m_path);
    m_thumb->setPath(m_path, Settings::get()->GUIThumbWidth);
    prepareThumbnailRows();
    endResetModel();
    emit directoryLoaded(m_path);
}

void FileModel::reloadParts()
{
    beginResetModel();
    PartCache::get()->refresh(m_path);
    setupColumns(m_path);
    m_thumb->setPath(m_path, Settings::get()->GUIThumbWidth);
    prepareThumbnailRows();
    endResetModel();
    m_prtReader->load(m_path, fileInfoList());
    emit directoryLoaded(m_path);
}

void FileModel::cancelThumbnails()
{
    m_thumb->cancelPending();
}

void FileModel::reloadThumbnails()
{
    m_thumb->clear();
    updateThumbnails();
}

void FileModel::settingsChanged()
{
    refreshModel();
}
void FileModel::moveParts(FileMover *mv)
{
    auto protectionSelection = PartSelector::get()->allSelectedIterator();
    while (protectionSelection.hasNext()) {
        protectionSelection.next();
        for (const auto &name : protectionSelection.value()) {
            const QFileInfo source(name);
            QString locked = DirectoryProtection::removalLock(source);
            const QFileInfo destination(QDir(Settings::get()->getWorkingDir()).filePath(source.fileName()));
            if (locked.isEmpty() && destination.exists())
                locked = DirectoryProtection::removalLock(destination);
            if (!locked.isEmpty()) {
                QMessageBox::warning(qobject_cast<QWidget *>(QObject::parent()),
                                     tr("Directory locked"), DirectoryProtection::message(locked));
                return;
            }
        }
    }

    QStringList clearList;
    QFileInfoList movedParts;
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

            LocalFilters filters;
            filters.load(dir, Settings::get()->ShowProeVersions);
            if (!filters.showVersions && !fi.isDir()
                    && File::versionedTypes().contains(FileMetadata(fi).type))
            {
                // When moving Pro/E files, we need to find all part versions
                mv->addSourceFiles(versionFamily(fi));

            } else {
                mv->addSourceFile(fi);
            }

            QString thumbPath = m_thumb->path(fi);

            if (!thumbPath.isEmpty())
                mv->addSourceFile(QFileInfo(thumbPath), THUMBNAILS_DIR);

            if (fi.isDir())
            {
                metaCache->clear(fname);
                pc->clearBelow(fname);
            }

            movedParts.append(fi);
        }

        clearList << dir;
    }

    mv->setDestination(dstDir);
    mv->work();
    for (const auto &source : movedParts) {
        const QFileInfo destination(QDir(dstDir).filePath(source.fileName()));
        if (!QFileInfo::exists(source.absoluteFilePath()) && destination.exists())
            metaCache->movePart(source.absolutePath(), File::partBaseName(source), dstDir);
    }

    selector->clear();

    foreach (const QString &dir, clearList)
        pc->clear(dir);

    pc->clear(dstDir);
}


void FileModel::deleteParts(DirectoryRemover *rm)
{
    auto protectionSelection = PartSelector::get()->allSelectedIterator();
    while (protectionSelection.hasNext()) {
        protectionSelection.next();
        for (const auto &name : protectionSelection.value()) {
            const QFileInfo source(name);
            QString locked = DirectoryProtection::removalLock(source);
            if (!locked.isEmpty()) {
                QMessageBox::warning(qobject_cast<QWidget *>(QObject::parent()),
                                     tr("Directory locked"), DirectoryProtection::message(locked));
                return;
            }
        }
    }

    QFileInfoList deleteList;
    QFileInfoList deletedParts;
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

            LocalFilters filters;
            filters.load(dir, Settings::get()->ShowProeVersions);
            if (!filters.showVersions && !fi.isDir()
                    && File::versionedTypes().contains(FileMetadata(fi).type))
            {
                // When deleting Pro/E files, we need to find all part versions
                deleteList << versionFamily(fi);

            } else {
                deleteList << fi;
            }

            if (fi.isDir())
            {
                MetadataCache::get()->clear(fname);
                pc->clearBelow(fname);
            }

            deletedParts.append(fi);
        }

        clearList << dir;
    }

    rm->addFiles(deleteList);
    rm->setStopOnError(false);
    rm->work();
    for (const auto &source : deletedParts) {
        if (!QFileInfo::exists(source.absoluteFilePath()))
            MetadataCache::get()->deletePart(source.absolutePath(), File::partBaseName(source));
    }

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

            metaCache->copyPart(key, File::partBaseName(fi), dstDir);
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
    const bool zima = (fi.type >= FileType::ZIMA_PRT && fi.type <= FileType::ZIMA_DRW)
            || fi.type == FileType::ZIMA_FORMAT || fi.type == FileType::ZIMA_TITLE_BLOCK;
    QString s = QString(":/gfx/icons/%1.%2").arg(File::getInternalNameForFileType(fi.type),
                                               zima ? "svg" : "png");

    if ((zima || File::versionedTypes().contains(fi.type)) && QFile::exists(s)) {
        auto found = m_cadIcons.constFind(s);
        if (found == m_cadIcons.cend())
            found = m_cadIcons.insert(s, QIcon(s));
        return found.value();
    }
    return QFileIconProvider::icon(info);
}

QString FileIconProvider::type ( const QFileInfo & info ) const
{
    return QFileIconProvider::type(info);
}
