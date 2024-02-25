#include "directoryeditparametersmodel.h"
#include <QGuiApplication>
#include <QFont>
#include <QBrush>
#include <QRegularExpression>
#include <QDebug>

DirectoryEditParametersModel::DirectoryEditParametersModel(QStringList parameters, QHash<QString, QString> labels, QObject *parent) :
	QAbstractListModel(parent),
	m_parameters(parameters),
	m_labels(labels)
{
}

int DirectoryEditParametersModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m_parameters.count();
}

int DirectoryEditParametersModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return 2;
}

QVariant DirectoryEditParametersModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int i = index.row();
	int col = index.column();
	QString param = m_parameters[i];

	if (col == 1)
	{
		if (role != Qt::DisplayRole)
			return QVariant();

		return m_parameters[i];
	}

	if (m_labels.contains(param) && !m_labels.value(param).isEmpty())
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return m_labels[param];

		default:
			return QVariant();
		}
	}

	switch (role)
	{
	case Qt::DisplayRole:
		return param;

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

QVariant DirectoryEditParametersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();

	switch (section)
	{
	case 0:
		return tr("Label");
	case 1:
		return tr("Handle");
	default:
		return QVariant();
	}
}

bool DirectoryEditParametersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(role)

	QString val = value.toString().trimmed();

	if (val.isEmpty())
		return false;

	switch (index.column())
	{
	case 0:
		m_labels[m_parameters[index.row()]] = val;
		return true;

	case 1: {
		if (hasParameter(val))
			return false;

		QString handle = m_parameters[index.row()];
		changeHandle(handle, val);
		emit parameterHandleChanged(handle, val);
		return true;
	}

	default:
		return false;
	}
}

bool DirectoryEditParametersModel::insertRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(count)

	beginInsertRows(parent, row, row);

	QString handle = newParameterHandle();
	m_parameters << handle;
	m_labels.insert(handle, tr("New column"));

	endInsertRows();

	return true;
}

bool DirectoryEditParametersModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (count != 1)
		return false;

	beginRemoveRows(parent, row, row);

	QString handle = m_parameters[row];

	m_parameters.removeAt(row);
	m_labels.remove(handle);

	endRemoveRows();

	return true;
}

Qt::ItemFlags DirectoryEditParametersModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags ret = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

	if (index.isValid())
		return Qt::ItemIsDragEnabled | ret;

	else
		return Qt::ItemIsDropEnabled | ret;
}

Qt::DropActions DirectoryEditParametersModel::supportedDropActions() const
{
	return Qt::CopyAction;
}

QStringList DirectoryEditParametersModel::mimeTypes() const
{
	QStringList types;
	types << "application/vnd.text.list";
	return types;
}

QMimeData *DirectoryEditParametersModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODeviceBase::WriteOnly);

	foreach (const QModelIndex &idx, indexes) {
		if (!idx.isValid())
			continue;

		QString text = data(
			index(idx.row(), 1, QModelIndex()),
			Qt::DisplayRole
		).toString();
		stream << text;
	}

	mimeData->setData("application/vnd.zcp.parameter.list", encodedData);
	return mimeData;
}

bool DirectoryEditParametersModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(action);
	Q_UNUSED(row);

	if (!data->hasFormat("application/vnd.zcp.parameter.list"))
		return false;

	if (parent.isValid() || column > 0)
		return false;

	return true;
}

bool DirectoryEditParametersModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (!canDropMimeData(data, action, row, column, parent))
		return false;

	if (action == Qt::IgnoreAction)
		return true;

	int beginRow;

	if (row != -1)
		beginRow = row;
	else
		beginRow = rowCount(QModelIndex());

	QByteArray encodedData = data->data("application/vnd.zcp.parameter.list");
	QDataStream stream(&encodedData, QIODeviceBase::ReadOnly);
	QStringList handles;

	while (!stream.atEnd())
	{
		QString handle;
		stream >> handle;
		handles << handle;
	}

	handles.removeDuplicates();

	int i = 0;

	foreach (const QString &handle, handles)
	{
		m_parameters.removeOne(handle);
		m_parameters.insert(beginRow + i++, handle);
	}

	emit parametersReordered(m_parameters);

	emit dataChanged(
		index(beginRow, 0, QModelIndex()),
		index(m_parameters.count() - 1, 1, QModelIndex())
	);

	return true;
}

QHash<QString, QString> DirectoryEditParametersModel::parameterLabels() const
{
	return m_labels;
}

bool DirectoryEditParametersModel::addParameter(const QString &handle)
{
	beginInsertRows(QModelIndex(), m_parameters.count(), m_parameters.count());
	m_parameters << handle;
	endInsertRows();
	return true;
}

bool DirectoryEditParametersModel::hasParameter(const QString &handle) const
{
	return m_parameters.contains(handle);
}

void DirectoryEditParametersModel::changeHandle(const QString &handle, const QString &newHandle)
{
	int i = m_parameters.indexOf(handle);
	QString label = m_labels[handle];

	m_labels.remove(handle);
	m_labels.insert(newHandle, label);

	m_parameters.replace(i, newHandle);

	QModelIndex idx = index(i, 1, QModelIndex());
	emit dataChanged(idx, idx);
}

void DirectoryEditParametersModel::removeParameter(const QString &handle)
{
	int i = m_parameters.indexOf(handle);

	beginRemoveRows(QModelIndex(), i, i);

	m_labels.remove(handle);
	m_parameters.removeOne(handle);

	endRemoveRows();
}

void DirectoryEditParametersModel::moveParameter(const QString &handle, int move)
{
	int currentIndex = m_parameters.indexOf(handle);
	int newIndex = currentIndex + move;
	QModelIndex firstIndex, lastIndex;

	if (currentIndex == -1 || move == 0)
		return;

	m_parameters.removeAt(currentIndex);
	m_parameters.insert(newIndex, handle);

	firstIndex = index(0, 0, QModelIndex());
	lastIndex = index(m_parameters.count() - 1, 1, QModelIndex());

	emit dataChanged(firstIndex, lastIndex);
	emit parametersReordered(m_parameters);
}

void DirectoryEditParametersModel::reorderParameters(const QStringList &parameters)
{
	if (m_parameters == parameters)
		return;

	m_parameters = parameters;

	emit dataChanged(
		index(0, 0, QModelIndex()),
		index(m_parameters.count() - 1, 1, QModelIndex())
	);
}

QString DirectoryEditParametersModel::newParameterHandle()
{
	QRegularExpression rx("param(\\d\\d)");
	int max = 0;

	foreach (const QString &param, m_parameters)
	{
		auto match = rx.match(param);

		if (!match.hasMatch())
			continue;

		int i = match.captured().toInt();

		if (i > max)
			max = i;
	}

	return QString("param%1").arg(max+1, 2, 10, QLatin1Char('0'));
}
