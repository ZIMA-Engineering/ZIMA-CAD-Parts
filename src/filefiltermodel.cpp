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
    File f(fm->fileInfo(current));

    if (f.type == File::UNDEFINED)
    {
        qDebug() << "FILTER6 undefined" << f.fileInfo.absoluteFilePath();
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
        qDebug() << "FILTER1" << QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    qDebug() << "FILTER2" << f.newestVersion << QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent) << (f.newestVersion && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent));
    return f.newestVersion && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
