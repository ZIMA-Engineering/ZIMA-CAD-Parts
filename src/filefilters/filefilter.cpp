#include "filefilter.h"

FileFilter::FileFilter(FileType::FileType type)
	: type(type)
{
}

void FileFilter::apply()
{
	enabled = item->checkState(0) == Qt::Checked;
	delete item;
}
