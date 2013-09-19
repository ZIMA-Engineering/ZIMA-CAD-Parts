#ifndef EXTENSIONFILTER_H
#define EXTENSIONFILTER_H

#include <QCheckBox>
#include "filefilter.h"

class ExtensionFilter : public FileFilter
{
public:
	ExtensionFilter(File::FileTypes type);
	FileFilters filterType();
	void load(QSettings *settings);
	void save(QSettings *settings);
	QWidget* widget();
	void apply();

private:
	QCheckBox *checkBox;
};

#endif // EXTENSIONFILTER_H
