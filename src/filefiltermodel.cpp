#include <QDebug>

#include "filefiltermodel.h"
#include "filemodel.h"
#include "file.h"
#include "settings.h"


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
		if (f.fileInfo.baseName() == METADATA_DIR)
			return false;

		return MetadataCache::get()->showDirectoriesAsParts(fm->path());
	}
	else if (!Settings::get()->filtersRegex.exactMatch(f.fileInfo.fileName()))
	{
		return false;
	}
	else if (f.type == FileType::UNDEFINED)
	{
		return false;
	}
	else if (m_showProeVersions)
	{
		// now we know that it's supported file and we should not take care about versions
		return isFiltered(fm->path(), f.fileInfo.baseName());
	}
	else if (File::versionedTypes().contains(f.type))
	{
		MetadataVersionsMap versions = MetadataCache::get()->partVersions(fm->path());

		return versions[f.fileInfo.completeBaseName()] == f.fileInfo.fileName()
			   && isFiltered(fm->path(), f.fileInfo.baseName());
	}

	return isFiltered(fm->path(), f.fileInfo.baseName());
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
