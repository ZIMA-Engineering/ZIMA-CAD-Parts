#include "filefilter.h"

FileFilter::FileFilter(File::FileType type)
	: type(type)
{
}

void FileFilter::apply()
{
	enabled = item->checkState(0) == Qt::Checked;
	delete item;
}
