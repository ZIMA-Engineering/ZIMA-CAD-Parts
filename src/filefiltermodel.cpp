#include "filefiltermodel.h"
#include "filemodel.h"
#include "localfilters.h"
#include "settings.h"

FileFilterModel::FileFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_showProeVersions = Settings::get()->ShowProeVersions;
    m_filterTimer.setSingleShot(true);
    m_filterTimer.setInterval(120);
    connect(&m_filterTimer, &QTimer::timeout, this, [this] { invalidateFilter(); });
}
void FileFilterModel::setSourceModel(QAbstractItemModel *model)
{
    if (sourceModel())
        disconnect(sourceModel(), nullptr, this, nullptr);
    m_prepared = false;
    if (model) {
        connect(model, &QAbstractItemModel::modelAboutToBeReset, this,
                [this] { m_prepared = false; m_filterTimer.stop(); });
        connect(model, &QAbstractItemModel::modelReset, this,
                [this] { m_prepared = false; });
    }
    QSortFilterProxyModel::setSourceModel(model);
}
void FileFilterModel::setShowProeVersions(bool show)
{
    m_showProeVersions = show;
    m_prepared = false;
    invalidateFilter();
}
void FileFilterModel::filterColumn(int column, const QString &text)
{
    if (text.isEmpty())
        m_filters.remove(column);
    else
        m_filters[column] = text;
    m_filterTimer.start();
}
void FileFilterModel::resetFilters()
{
    m_filterTimer.stop();
    m_filters.clear();
    m_prepared = false;
    invalidateFilter();
}
bool FileFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    auto model = qobject_cast<FileModel *>(sourceModel());
    if (!model || model->path().isEmpty())
        return false;
    if (!m_prepared) {
        LocalFilters filters;
        filters.load(model->path(), m_showProeVersions);
        m_accepted = filters.accepted(model->fileInfoList(),
            MetadataCache::get()->showDirectoriesAsParts(model->path()));
        m_prepared = true;
    }
    if (row < 0 || row >= m_accepted.size() || !m_accepted.testBit(row))
        return false;
    for (auto it = m_filters.cbegin(); it != m_filters.cend(); ++it) {
        // Use displayed values, including the complete filename and the correct
        // metadata handle. No thumbnail requests or file-type detection here.
        if (!model->data(model->index(row, it.key(), parent), Qt::DisplayRole)
                .toString().contains(it.value(), Qt::CaseInsensitive))
            return false;
    }
    return true;
}
