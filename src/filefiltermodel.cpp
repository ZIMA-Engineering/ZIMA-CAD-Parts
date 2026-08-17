#include <QDebug>

#include "filefiltermodel.h"
#include "filemodel.h"
#include "file.h"
#include "settings.h"
#include "partcache.h"

#include <optional>
#include <algorithm>


FileFilterModel::FileFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_showProeVersions(true)
{
    setShowProeVersions(Settings::get()->ShowProeVersions);
}

void FileFilterModel::setShowProeVersions(bool show)
{
    m_showProeVersions = show;
    invalidate();
}

void FileFilterModel::setDirectory(const QString &directory)
{
    m_directory = directory;
    m_config = FileFilterConfig::load(directory);
    invalidateFilter();
}

void FileFilterModel::filterColumn(int column, const QString &text)
{
    if (text.isEmpty())
        m_filters.remove(column);

    else
        m_filters[column] = text;

    beginResetModel();
    endResetModel();
}

void FileFilterModel::resetFilters()
{
    if (m_filters.empty())
        return;

    m_filters.clear();
    beginResetModel();
    endResetModel();
}

bool FileFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    FileModel *fm = qobject_cast<FileModel*>(sourceModel());
    Q_ASSERT(fm);

    QModelIndex current = fm->index(source_row, 0, source_parent);
    FileMetadata f(fm->fileInfo(current));

    if (f.fileInfo.isDir())
    {
        if (f.fileInfo.fileName().compare(METADATA_DIR, Qt::CaseInsensitive) == 0)
            return false;

        return MetadataCache::get()->showDirectoriesAsParts(fm->path())
               && isFiltered(fm->path(), File::partBaseName(f.fileInfo));
    }
    const FileFilterMatch configMatch = m_config.match(f.fileInfo.fileName());
    if (!configMatch.accepted)
        return false;

    if (m_config.configured)
    {
        if (!configMatch.showVersions && configMatch.versionMode == FileVersionMode::ZimaCad
                && configMatch.version >= 0)
            return false;

        if (!configMatch.showVersions && configMatch.versionMode == FileVersionMode::ProE)
        {
            int highestVersion = configMatch.version;
            for (const QFileInfo &candidate : PartCache::get()->parts(fm->path()))
            {
                const FileFilterMatch candidateMatch = m_config.match(candidate.fileName());
                if (candidateMatch.accepted
                        && candidateMatch.versionMode == FileVersionMode::ProE
                        && candidateMatch.logicalName.compare(configMatch.logicalName,
                                                              Qt::CaseInsensitive) == 0)
                    highestVersion = std::max(highestVersion, candidateMatch.version);
            }
            if (configMatch.version != highestVersion)
                return false;
        }

        return isFiltered(fm->path(), File::partBaseName(f.fileInfo));
    }

    // Sources without files.ini are intentionally permissive for backwards compatibility.
    return isFiltered(fm->path(), File::partBaseName(f.fileInfo));
}

bool FileFilterModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    Q_UNUSED(source_parent);
    Q_UNUSED(source_column)
    // hide all QFileSystemModel "meta" columns and use only extended ones
//    if (source_column >=1 && source_column < 4)
//        return false;
    return true;
}

struct VersionedNameInfo
{
    QString baseName;
    int version;
    FileType::FileType type;
};

static std::optional<VersionedNameInfo> parseVersionedName(const QString &fileName)
{
    foreach (FileType::FileType t, File::versionedTypes())
    {
        QRegularExpression re(File::getRxForFileType(t), QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(fileName);

        if (match.hasMatch())
        {
            VersionedNameInfo info;
            info.type = t;
            info.baseName = match.captured(2);
            info.version = match.captured(3).toInt();
            return info;
        }
    }

    return std::nullopt;
}

bool FileFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() == 0 && right.column() == 0)
    {
        FileModel *fm = qobject_cast<FileModel*>(sourceModel());

        if (fm)
        {
            QFileInfo lfi = fm->fileInfo(left);
            QFileInfo rfi = fm->fileInfo(right);

            auto lInfo = parseVersionedName(lfi.fileName());
            auto rInfo = parseVersionedName(rfi.fileName());

            if (lInfo.has_value() && rInfo.has_value()
                    && lInfo->baseName == rInfo->baseName
                    && lInfo->type == rInfo->type)
            {
                if (lInfo->version != rInfo->version)
                    return lInfo->version < rInfo->version;
            }
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool FileFilterModel::isFiltered(const QString &path, const QString &name) const
{
    if (m_filters.empty())
        return true;

    auto meta = MetadataCache::get();
    QMap<int, QString>::const_iterator i = m_filters.constBegin();

    while (i != m_filters.constEnd()) {
        int col = i.key();
        const QString &val = i.value();

        if (col == 0) {
            // Part name
            if (!name.contains(val))
                return false;

        } else if (col == 1) {
            // Thumbnail
            // This shouldn't happen, do nothing

        } else {
            QString data = meta->partParam(path, name, col-2);

            if (!data.contains(val))
                return false;
        }

        ++i;
    }

    return true;
}
