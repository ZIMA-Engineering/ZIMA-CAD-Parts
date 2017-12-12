#include "partcache.h"

#include <QDir>

PartCache* PartCache::m_instance = nullptr;

PartCache *PartCache::get()
{
	if (!m_instance)
		m_instance = new PartCache;

	return m_instance;
}

QFileInfoList PartCache::parts(const QString &dir)
{
	if (m_parts.contains(dir))
		return m_parts.value(dir);

	QDir d(dir);
	auto list = d.entryInfoList(
		QDir::Files
		| QDir::Dirs
		| QDir::Readable
		| QDir::NoDotAndDotDot,
		QDir::Name
	);

	m_parts.insert(dir, list);
	return list;
}

int PartCache::count(const QString &dir)
{
	return parts(dir).count();
}

QFileInfo PartCache::partAt(const QString &dir, int index)
{
	return parts(dir).at(index);
}

void PartCache::clear(const QString &dir)
{
	if (!m_parts.contains(dir))
		return;

	m_parts.remove(dir);
	emit cleared(dir);
}

void PartCache::renameDirectory(const QString &oldDir, const QString &newDir)
{
	if (!m_parts.contains(oldDir))
		return;

	m_parts.insert(newDir, m_parts[oldDir]);
	m_parts.remove(oldDir);
	emit directoryRenamed(oldDir, newDir);
}

PartCache::PartCache()
{

}
