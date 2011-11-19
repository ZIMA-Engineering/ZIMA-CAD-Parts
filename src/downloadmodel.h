#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QList>
#include <QSettings>
#include "item.h"

class DownloadDelegate : public QItemDelegate
{
public:
	DownloadDelegate(QObject* parent=0) : QItemDelegate(parent) {}
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class DownloadModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit DownloadModel(QObject *parent = 0);

	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:

public slots:
	void setQueue(QList<File*> *q);
	void fileChanged(File *file);
	void fileDownloaded(File *file);
	void queueChanged();

private:
	QList<File*> *queue;

};

#endif // DOWNLOADMODEL_H
