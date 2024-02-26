#include "extensionfilter.h"

ExtensionFilter::ExtensionFilter(FileType::FileType type)
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

QTreeWidgetItem* ExtensionFilter::widget()
{
    item = new QTreeWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setText(0, File::getLabelForFileType(type));
    item->setCheckState(0, enabled ? Qt::Checked : Qt::Unchecked);
    item->setIcon(0, QIcon(QString(":/gfx/icons/%1.png").arg(File::getInternalNameForFileType(type))));

    return item;
}
