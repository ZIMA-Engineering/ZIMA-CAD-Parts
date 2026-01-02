#ifndef FILEFILTERMODEL_H
#define FILEFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <QMap>

class FileFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileFilterModel(QObject *parent = 0);
    void setShowProeVersions(bool show);

public slots:
    void filterColumn(int column, const QString &text);
    void resetFilters();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    bool m_showProeVersions;
    QMap<int, QString> m_filters;

    bool isFiltered(const QString &path, const QString &name) const;

};

#endif // FILEFILTERMODEL_H
