#include "metadatamigration.h"

MetadataMigration::MetadataMigration()
{

}

MetadataMigration::~MetadataMigration()
{

}

void MetadataMigration::setSettings(QSettings *settings)
{
    m_settings = settings;
}

bool MetadataMigration::hasGroup(const QString &name)
{
    return m_settings->childGroups().contains(name);
}
