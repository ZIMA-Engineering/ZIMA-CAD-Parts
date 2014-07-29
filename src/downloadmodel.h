/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QList>

class DataTransfer;
class TransferHandler;
struct File;

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
	enum TransferHandlerType {
	    ServersModel,
	    TechSpec,
	    None
	};

	explicit DownloadModel(TransferHandler *handler, QObject *parent = 0);

	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QList<File*> files();
	bool isEmpty() const;
	bool isDownloading() const;

signals:

public slots:
	void enqueue(File *f);
	void enqueue(QList<File*> list);
	void clear();
	void fileChanged(File *file);
	void fileDownloaded(File *file);
	void stop();
	void resume();

private slots:


private:
	QList<File*> queue;
	bool m_downloading;
	TransferHandler *m_handler;
};

#endif // DOWNLOADMODEL_H
