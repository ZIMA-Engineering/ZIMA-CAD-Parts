#ifndef VERSIONFILTER_H
#define VERSIONFILTER_H

#include <QCheckBox>

#include "filefilter.h"

class VersionFilter : public FileFilter
{
public:
	VersionFilter();
	FileFilters filterType();
	void load(QSettings *settings);
	void save(QSettings *settings);
	QWidget* widget();
	void apply();

private:
	QCheckBox *checkBox;
};

#endif // VERSIONFILTER_H
