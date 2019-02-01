#include "partcache.h"
#include "metadata.h"

#include <QDir>
#include <QSet>
#include <QDebug>

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

	auto list = listFiles(dir);
	m_parts.insert(dir, list);

	if (!m_fsWatcher.addPath(dir))
		qDebug() << "Unable to watch directory" << dir << "for changes";

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

void PartCache::refresh(const QString &dir)
{
	if (!m_parts.contains(dir))
		return;

	m_parts.remove(dir);
	MetadataCache::get()->clearPartVersions(dir);
}

void PartCache::clear(const QString &dir)
{
	if (!m_parts.contains(dir))
		return;

	m_fsWatcher.removePath(dir);
	m_parts.remove(dir);
	MetadataCache::get()->clearPartVersions(dir);
	emit cleared(dir);
}

void PartCache::renameDirectory(const QString &oldDir, const QString &newDir)
{
	if (!m_parts.contains(oldDir))
		return;

	m_fsWatcher.removePath(oldDir);
	m_parts.insert(newDir, m_parts[oldDir]);
	m_parts.remove(oldDir);
	m_fsWatcher.addPath(newDir);
	emit directoryRenamed(oldDir, newDir);
}

PartCache::PartCache()
{
	connect(&m_fsWatcher, SIGNAL(directoryChanged(QString)),
			this, SLOT(onDirectoryChange(QString)));
}

QFileInfoList PartCache::listFiles(const QString &dir)
{
	QDir d(dir);
	return d.entryInfoList(
		QDir::Files
		| QDir::Dirs
		| QDir::Readable
		| QDir::NoDotAndDotDot,
		QDir::Name
	);
}

void PartCache::processDiff(const QString &dir, const QFileInfoList &newFiles)
{
	const QFileInfoList oldFiles = m_parts.value(dir);

	QSet<QFileInfo> removed = oldFiles.toSet().subtract(newFiles.toSet());
	QSet<QFileInfo> added = newFiles.toSet().subtract(oldFiles.toSet());

	foreach (const QFileInfo &fi, removed) {
		qDebug() << "Part" << fi.baseName() << "removed from" << dir;
		emit partRemoved(dir, fi);
	}

	foreach (const QFileInfo &fi, added) {
		qDebug() << "Found new part" << fi.baseName() << "in" << dir;
		emit partAdded(dir, fi);
	}

	m_parts.insert(dir, newFiles);

	if (!removed.empty() || !added.empty())
	{
		MetadataCache::get()->clearPartVersions(dir);
		emit directoryChanged(dir);
	}
}

void PartCache::onDirectoryChange(const QString &path)
{
	if (!m_parts.contains(path))
		return;

	if (!QFile::exists(path)) {
		clear(path); // TODO we should signal the directory no longer exists
		return;
	}

	processDiff(path, listFiles(path));
}
