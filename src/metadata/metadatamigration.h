#ifndef METADATAMIGRATION_H
#define METADATAMIGRATION_H

#include <QSettings>

class MetadataMigration
{
public:
    MetadataMigration();
    virtual ~MetadataMigration();
    void setSettings(QSettings *settings);
    virtual bool migrate() = 0;

protected:
    QSettings *m_settings;

    bool hasGroup(const QString &name);
};

#endif // METADATAMIGRATION_H
