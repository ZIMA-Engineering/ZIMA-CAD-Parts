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

QWidget* VersionFilter::widget()
{
	checkBox = new QCheckBox(QObject::tr("Show versions"));
	checkBox->setChecked(enabled);

	return checkBox;
}

void VersionFilter::apply()
{
	enabled = checkBox->isChecked();
	checkBox->deleteLater();
}

