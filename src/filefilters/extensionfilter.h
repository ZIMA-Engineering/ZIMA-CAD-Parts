#ifndef EXTENSIONFILTER_H
#define EXTENSIONFILTER_H

#include <QCheckBox>
#include "filefilter.h"

class ExtensionFilter : public FileFilter
{
public:
    ExtensionFilter(FileType::FileType type);
	FileFilters filterType();
	void load(QSettings *settings);
	void save(QSettings *settings);
	QTreeWidgetItem* widget();
};

#endif // EXTENSIONFILTER_H
