#ifndef FILEFILTER_H
#define FILEFILTER_H

#include <QSettings>

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
	virtual QWidget* widget() = 0;
	virtual void apply() = 0;

	File::FileTypes type;
	bool enabled;
	FilterGroup *group;
};

#endif // FILEFILTER_H
