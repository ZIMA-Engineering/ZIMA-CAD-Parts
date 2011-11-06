#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractItemModel>

class Item;
class File;

class FileModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit FileModel(QObject *parent = 0);

	//---
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
	QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
	QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	//---

	Item* getRootItem();
	void setThumbWidth(int size);
	void setPreviewWidth(int size);

public slots:
	void setRootIndex(const QModelIndex &index);

protected:
	Item* rootItem;

private:
	int thumbWidth;
	int previewWidth;

private slots:
	void thumbnailDownloaded(File *file);

signals:

};

#endif // FILEMODEL_H
