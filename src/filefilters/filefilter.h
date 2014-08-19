#ifndef FILEFILTER_H
#define FILEFILTER_H

#include <QSettings>
#include <QTreeWidgetItem>

#include "filtergroup.h"
#include "file.h"

class FilterGroup;


class FileFilter
{
public:
	enum FileFilters {
		Extension,
		Version
	};

    FileFilter(FileType::FileType type);
	virtual FileFilters filterType() = 0;
	virtual void load(QSettings *settings) = 0;
	virtual void save(QSettings *settings) = 0;
	virtual QTreeWidgetItem* widget() = 0;

	virtual void apply();

    FileType::FileType type;
	bool enabled;
	FilterGroup *group;

protected:
	QTreeWidgetItem *item;
};

#endif // FILEFILTER_H
