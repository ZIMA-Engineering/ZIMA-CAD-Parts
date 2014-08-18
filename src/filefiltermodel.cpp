#include <QDebug>

#include "filefiltermodel.h"
#include "filemodel.h"
#include "file.h"


FileFilterModel::FileFilterModel(QObject *parent) :
	QSortFilterProxyModel(parent),
	m_showProeVersions(true)
{
}

void FileFilterModel::setShowProeVersions(bool show)
{
	m_showProeVersions = show;
}

bool FileFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    FileModel *fm = qobject_cast<FileModel*>(sourceModel());
    Q_ASSERT(fm);

    QModelIndex current = fm->index(source_row, 0, source_parent);
    FileMetadata f(fm->fileInfo(current));

    if (f.fileInfo.isDir())
        return true;
    else if (f.type == File::UNDEFINED)
    {
        return false;
    }
    else if (m_showProeVersions)
    {
        // now we know that it's supported file and we should not take care about versions
        return true;
    }

    if (!f.version
            || (
                   f.type != File::PRT_PROE
                && f.type != File::ASM
                && f.type != File::DRW
                && f.type != File::FRM
                && f.type != File::NEU_PROE)
            )
    {
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    return f.newestVersion && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool FileFilterModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    Q_UNUSED(source_parent);
    // hide all QFileSystemModel "meta" columns and use only extended ones
    if (source_column >=1 && source_column < 4)
        return false;
    return true;
}
