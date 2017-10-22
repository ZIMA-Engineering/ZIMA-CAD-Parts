#ifndef DIRECTORYEDITPARAMETERSMODEL_H
#define DIRECTORYEDITPARAMETERSMODEL_H

#include <QAbstractListModel>
#include <QHash>

class DirectoryEditParametersModel : public QAbstractListModel
{
Q_OBJECT

public:
	DirectoryEditParametersModel(QStringList parameters, QHash<QString, QString> labels, QObject *parent = nullptr);
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	bool insertRows(int row, int count, const QModelIndex &parent);
	bool removeRows(int row, int count, const QModelIndex &parent);
	Qt::ItemFlags flags(const QModelIndex &index) const;

	QHash<QString, QString> parameterLabels() const;
	bool addParameter(const QString &handle);
	bool hasParameter(const QString &handle) const;
	void changeHandle(const QString &handle, const QString &newHandle);
	void removeParameter(const QString &handle);

signals:
	void parameterHandleChanged(const QString &handle, const QString &newHandle);

private:
	QStringList m_parameters;
	QHash<QString, QString> m_labels;

	QString newParameterHandle();
};

#endif // DIRECTORYEDITPARAMETERSMODEL_H
