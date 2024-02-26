#ifndef METADATAMIGRATOR_H
#define METADATAMIGRATOR_H

#include <QSettings>
#include <QHash>

class MetadataMigration;

class MetadataMigrator
{
public:
    MetadataMigrator(QSettings *settings);
    ~MetadataMigrator();
    bool migrate(int from, int to);

private:
    QSettings *m_settings;
    QHash<int, MetadataMigration*> m_migrations;

    void setupMigrations();
    bool backup(const QString &file, int version);
};

#endif // METADATAMIGRATOR_H
