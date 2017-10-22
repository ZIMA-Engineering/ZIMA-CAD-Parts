#include "metadatamigrator.h"

#include "metadatamigration.h"
#include "migrations/metadatav2migration.h"

#include <QFile>
#include <QDebug>

MetadataMigrator::MetadataMigrator(QSettings *settings)
	: m_settings(settings)
{
	setupMigrations();
}

MetadataMigrator::~MetadataMigrator()
{
	QHashIterator<int, MetadataMigration*> i(m_migrations);

	while (i.hasNext())
	{
		i.next();
		delete i.value();
	}
}

bool MetadataMigrator::migrate(int from, int to)
{
	for (int v = from+1; v <= to; v++)
	{
		if (!m_migrations.contains(v))
		{
			qDebug() << "Migration to version" << v << "not defined";
			return false;
		}

		if (!backup(m_settings->fileName(), v-1))
		{
			qDebug() << "Unable to create a backup of version" << v-1;
			return false;
		}

		if (!m_migrations[v]->migrate())
		{
			qDebug() << "Migration to version" << v << "failed";
			return false;
		}
	}

	return true;
}

void MetadataMigrator::setupMigrations()
{
	auto m = new MetadataV2Migration;
	m->setSettings(m_settings);

	m_migrations.insert(2, m);
}

bool MetadataMigrator::backup(const QString &file, int version)
{
	return QFile::copy(file, QString("%1.v%2").arg(file).arg(version));
}
