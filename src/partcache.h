#ifndef PARTCACHE_H
#define PARTCACHE_H

#include <QObject>
#include <QFileInfoList>

class PartCache : public QObject
{
	Q_OBJECT
public:
	static PartCache *get();
	QFileInfoList parts(const QString &dir);
	int count(const QString &dir);
	QFileInfo partAt(const QString &dir, int index);
	void clear(const QString &dir);
	void renameDirectory(const QString &oldDir, const QString &newDir);

signals:
	void cleared(const QString &dir);
	void directoryRenamed(const QString &oldDir, const QString &newDir);

private:
	static PartCache *m_instance;
	QHash<QString, QFileInfoList> m_parts;

	PartCache();

};

#endif // PARTCACHE_H
