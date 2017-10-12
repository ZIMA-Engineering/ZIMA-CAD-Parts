#ifndef DIRECTORYEDITCOLUMNMODEL_H
#define DIRECTORYEDITCOLUMNMODEL_H

#include <QAbstractListModel>

class DirectoryEditColumnModel : public QAbstractListModel
{
public:
	DirectoryEditColumnModel(QStringList columnLabels, QStringList primaryColumns, QObject *parent = nullptr);
	int rowCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QStringList columnLabels() const;

private:
	QStringList m_columns;
	QStringList m_primaryColumns;
};

#endif // DIRECTORYEDITCOLUMNMODEL_H
