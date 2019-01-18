#ifndef PARTCACHE_H
#define PARTCACHE_H

#include <QObject>
#include <QFileInfoList>
#include <QFileSystemWatcher>

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
	void directoryChanged(const QString &dir);
	void partAdded(const QString &dir, const QFileInfo &part);
	void partRemoved(const QString &dir, const QFileInfo &part);

private:
	static PartCache *m_instance;
	QHash<QString, QFileInfoList> m_parts;
	QFileSystemWatcher m_fsWatcher;

	PartCache();
	QFileInfoList listFiles(const QString &dir);
	void processDiff(const QString &dir, const QFileInfoList &newFiles);

private slots:
	void onDirectoryChange(const QString &path);
};

inline uint qHash(const QFileInfo &key, uint seed)
{
	return qHash(key.absoluteFilePath(), seed);
}

#endif // PARTCACHE_H
