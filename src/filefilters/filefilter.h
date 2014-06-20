#ifndef FILEFILTER_H
#define FILEFILTER_H

#include <QSettings>
#include <QTreeWidgetItem>

#include "../item.h"
#include "filtergroup.h"

class FilterGroup;

class FileFilter
{
public:
	enum FileFilters {
		Extension,
		Version
	};

	FileFilter(File::FileTypes type);
	virtual FileFilters filterType() = 0;
	virtual void load(QSettings *settings) = 0;
	virtual void save(QSettings *settings) = 0;
	virtual QTreeWidgetItem* widget() = 0;

	virtual void apply();

	File::FileTypes type;
	bool enabled;
	FilterGroup *group;

protected:
	QTreeWidgetItem *item;
};

#endif // FILEFILTER_H
