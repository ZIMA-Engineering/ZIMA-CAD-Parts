#include "versionfilter.h"

VersionFilter::VersionFilter()
	: FileFilter(File::UNDEFINED)
{

}

VersionFilter::FileFilters VersionFilter::filterType()
{
	return Version;
}

void VersionFilter::load(QSettings *settings)
{
	enabled = settings->value("versions", true).toBool();
}

void VersionFilter::save(QSettings *settings)
{
	settings->setValue("versions", enabled);
}

QTreeWidgetItem* VersionFilter::widget()
{
	item = new QTreeWidgetItem();
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setText(0, QObject::tr("Show versions"));
	item->setCheckState(0, enabled ? Qt::Checked : Qt::Unchecked);

	return item;
}
