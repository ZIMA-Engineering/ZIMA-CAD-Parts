#ifndef FILEFILTERMODEL_H
#define FILEFILTERMODEL_H

#include <QSortFilterProxyModel>

class FileFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit FileFilterModel(QObject *parent = 0);
	void setShowProeVersions(bool show);
	
protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
	bool m_showProeVersions;
	
};

#endif // FILEFILTERMODEL_H
