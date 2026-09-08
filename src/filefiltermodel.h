#ifndef FILEFILTERMODEL_H
#define FILEFILTERMODEL_H
#include <QSortFilterProxyModel>
#include <QMap>
#include <QBitArray>
#include <QTimer>

class FileFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileFilterModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *model) override;
    void setShowProeVersions(bool show);
public slots:
    void filterColumn(int column, const QString &text);
    void resetFilters();
protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
private:
    bool m_showProeVersions = true;
    QMap<int, QString> m_filters;
    QTimer m_filterTimer;
    mutable bool m_prepared = false;
    mutable QBitArray m_accepted;
};
#endif
