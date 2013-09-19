#include "extensionfilter.h"

ExtensionFilter::ExtensionFilter(File::FileTypes type)
	: FileFilter(type)
{

}

FileFilter::FileFilters ExtensionFilter::filterType()
{
	return Extension;
}

void ExtensionFilter::load(QSettings *settings)
{
	enabled = settings->value(File::getInternalNameForFileType(type), true).toBool();
}

void ExtensionFilter::save(QSettings *settings)
{
	settings->setValue(File::getInternalNameForFileType(type), enabled);
}

QWidget* ExtensionFilter::widget()
{
	checkBox = new QCheckBox(File::getLabelForFileType(type));
	checkBox->setChecked(enabled);
	checkBox->setIcon(QIcon(QString(":/gfx/icons/%1.png").arg(File::getInternalNameForFileType(type))));

	return checkBox;
}

void ExtensionFilter::apply()
{
	enabled = checkBox->isChecked();
	checkBox->deleteLater();
}
