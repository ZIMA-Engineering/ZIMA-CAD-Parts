#include <QDebug>

#include "filefiltermodel.h"
#include "item.h"
#include "filemodel.h"

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
	if(m_showProeVersions)
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	Item *item = static_cast<FileModel*>(sourceModel())->getRootItem();
	File *f = item->files.at(source_row);

	if(!f->version || (
		   f->type != File::PRT_PROE
		&& f->type != File::ASM
		&& f->type != File::DRW
		&& f->type != File::FRM
		&& f->type != File::NEU_PROE)
	)
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	qDebug() << "File filter model does magic!" << source_row << f->name;

	return f->newestVersion && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
