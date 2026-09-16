#include "core/partsread.h"
#include "partcache.h"
#include "metadata.h"

#include <QDir>
#include <QSet>
#include <QDebug>

namespace {
bool coversDirectory(const QString &root, const QString &path)
{
#ifdef Q_OS_WIN
    const auto sensitivity = Qt::CaseInsensitive;
#else
    const auto sensitivity = Qt::CaseSensitive;
#endif
    const auto directory = QDir::cleanPath(root);
    const auto candidate = QDir::cleanPath(path);
    return candidate.compare(directory, sensitivity) == 0
        || candidate.startsWith(directory.endsWith('/') ? directory : directory + '/', sensitivity);
}
}
PartCache* PartCache::m_instance = nullptr;

PartCache *PartCache::get()
{
    if (!m_instance)
        m_instance = new PartCache;

    return m_instance;
}

QFileInfoList PartCache::parts(const QString &dir)
{
    if (dir.trimmed().isEmpty() || !QFileInfo(dir).isDir())
        return QFileInfoList();

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
    // Refresh must not delete user metadata, including legacy file-name keys.
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

void PartCache::clearBelow(const QString &path)
{
    const QString cleanPath = QDir::cleanPath(path);
    const QString cleanPathWithSlash = cleanPath + QLatin1Char('/');
    QStringList dirs;
#ifdef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif

    QHashIterator<QString, QFileInfoList> it(m_parts);
    while (it.hasNext())
    {
        it.next();

        const QString dir = QDir::cleanPath(it.key());
        if (dir.compare(cleanPath, sensitivity) == 0
                || dir.startsWith(cleanPathWithSlash, sensitivity))
            dirs << it.key();
    }

    foreach (const QString &dir, dirs)
        clear(dir);
}

void PartCache::renameDirectory(const QString &oldDir, const QString &newDir)
{
    if (!m_parts.contains(oldDir))
        return;

    m_fsWatcher.removePath(oldDir);
    m_parts.insert(newDir, listFiles(newDir));
    m_parts.remove(oldDir);
    if (!newDir.trimmed().isEmpty() && QFileInfo(newDir).isDir())
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
    return PartsCore::listEntries(dir);
}

void PartCache::processDiff(const QString &dir, const QFileInfoList &newFiles)
{
    const QFileInfoList oldFiles = m_parts.value(dir);

    QSet<QFileInfo> removed(oldFiles.begin(), oldFiles.end());
    removed.subtract(QSet<QFileInfo>(newFiles.begin(), newFiles.end()));

    QSet<QFileInfo> added(newFiles.begin(), newFiles.end());
    added.subtract(QSet<QFileInfo>(oldFiles.begin(), oldFiles.end()));

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

void PartCache::beginFileChanges(const QString &directory)
{
    const auto path = QDir::cleanPath(directory);
    if (++m_fileChanges[path] == 1) emit directoryOperationStarted(path);
}

void PartCache::endFileChanges(const QString &directory)
{
    const auto path = QDir::cleanPath(directory);
    auto found = m_fileChanges.find(path);
    if (found == m_fileChanges.end() || --found.value() > 0) return;
    m_fileChanges.erase(found);
    const auto cached = m_parts.keys();
    for (const auto &entry : cached)
        if (coversDirectory(path, entry)) onDirectoryChange(entry);
    emit directoryOperationFinished(path, path);
}

void PartCache::onDirectoryChange(const QString &path)
{
    for (auto it = m_fileChanges.cbegin(); it != m_fileChanges.cend(); ++it)
        if (coversDirectory(it.key(), path)) return;
    if (!m_parts.contains(path))
        return;

    if (!QFile::exists(path)) {
        clear(path); // TODO we should signal the directory no longer exists
        return;
    }

    processDiff(path, listFiles(path));
}
