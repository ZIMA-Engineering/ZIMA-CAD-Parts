#include "directoryeditcolumnmodel.h"
#include <QGuiApplication>
#include <QFont>
#include <QBrush>
#include <QDebug>

DirectoryEditColumnModel::DirectoryEditColumnModel(QStringList columnLabels, QStringList primaryColumns, QObject *parent) :
	QAbstractListModel(parent),
	m_columns(columnLabels),
	m_primaryColumns(primaryColumns)
{
}

int DirectoryEditColumnModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m_primaryColumns.count();
}

QVariant DirectoryEditColumnModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int i = index.row();

	if (i < m_columns.count())
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return m_columns[i];

		default:
			return QVariant();
		}
	}

	switch (role)
	{
	case Qt::DisplayRole:
		return m_primaryColumns[i];

	case Qt::FontRole: {
		auto font = QGuiApplication::font();
		font.setItalic(true);
		return font;
	}

	case Qt::ForegroundRole:
		// TODO: use some system color
		return QBrush(Qt::gray);

	default:
		return QVariant();
	}
}

QVariant DirectoryEditColumnModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(section)
	Q_UNUSED(orientation)
	Q_UNUSED(role)

	return QVariant();
}

bool DirectoryEditColumnModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	int i = index.row();
	int cnt = m_columns.count();

	if (i > cnt || role != Qt::EditRole)
		return false;

	QString val = value.toString().trimmed();

	if (val.isEmpty())
		return false;

	if (i < cnt)
		m_columns[i] = val;

	else
		m_columns << val;

	return true;
}

Qt::ItemFlags DirectoryEditColumnModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags ret = QAbstractItemModel::flags(index);

	if (index.row() <= m_columns.count())
		ret |= Qt::ItemIsEditable;

	return ret;
}

QStringList DirectoryEditColumnModel::columnLabels() const
{
	return m_columns;
}
