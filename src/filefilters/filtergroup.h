#ifndef FILTERGROUP_H
#define FILTERGROUP_H

#include <QGroupBox>

#include "filefilter.h"

class FileFilter;

class FilterGroup
{
public:
    FilterGroup(QString internalName, QString label) : label(label), internalName(internalName), enabled(0){}
	FilterGroup& operator<<(FileFilter* f);
	QWidget* widget();
	void apply();

	QString label;
	QString internalName;
	QList<FileFilter*> filters;
	bool enabled;

private:
	QGroupBox *groupBox;
};

#endif // FILTERGROUP_H
