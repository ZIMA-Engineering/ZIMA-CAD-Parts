#include "thumbnailmanager.h"
#include "settings.h"
#include <QtDebug>

ThumbnailWorker::ThumbnailWorker(const QString &path)
{
    m_path = path;
}

void ThumbnailWorker::run()
{
    qDebug() << "Starting thread";

    ThumbnailMap ret;

    Metadata* m = MetadataCache::get()->metadata(m_path);
    getThumbs(m, &ret);

    emit dataReady(ret);

    qDebug() << "Thread finished";
}

void ThumbnailWorker::getThumbs(Metadata *m, ThumbnailMap* map)
{
    QString path = m->path();

    QDir d(path + "/" + THUMBNAILS_DIR);
    QStringList thumbs = d.entryList(QStringList() << "*.png" << "*.jpg",
                                     QDir::Files | QDir::Readable);

    QString pmPath;
    QFileInfo fi;

    foreach (QString i, thumbs)
    {
        fi.setFile(i);
        if (map->contains(fi.baseName()))
            continue;
        pmPath = path + "/" + THUMBNAILS_DIR + "/" + i;
        map->insert(fi.baseName(), qMakePair(pmPath, QPixmap(pmPath).scaled(Settings::get()->GUIThumbWidth,
                                                                            Settings::get()->GUIThumbWidth,
                                                                            Qt::KeepAspectRatio)));
    }

    // now includes
    foreach(Metadata *i, m->includes())
    {
        getThumbs(i, map);
    }
}


ThumbnailManager::ThumbnailManager(QObject *parent)
    : QObject(parent),
      m_worker(0)
{
    qRegisterMetaType<ThumbnailMap>("ThumbnailMap");

    m_loading = QPixmap(":/gfx/image-loading.png");
}

void ThumbnailManager::setPath(const QString &path)
{
    m_path = path;
    clear();
}

void ThumbnailManager::clear()
{
    if (m_worker)
    {
        m_worker->quit();
        m_worker->deleteLater();
        m_worker = 0;
    }
    m_cache.clear();
    load();
}

void ThumbnailManager::load()
{
    m_worker = new ThumbnailWorker(m_path);
    connect(m_worker, SIGNAL(dataReady(ThumbnailMap)), this, SLOT(dataReady(ThumbnailMap)));
    // TODO/FIXME: sig/slots
    m_worker->start();
}

void ThumbnailManager::dataReady(const ThumbnailMap &data)
{
    m_cache = data;
    qDebug() << m_cache;
    emit updateModel();
}

QPixmap ThumbnailManager::thumbnail(const QFileInfo &fi)
{
    if (!m_cache.size())
        return m_loading;
    else if (m_cache.contains(fi.baseName()) && !m_cache[fi.baseName()].second.isNull())
        return m_cache[fi.baseName()].second;
    else
        return QPixmap();
}

QString ThumbnailManager::tooltip(const QFileInfo &fi)
{
    if (!m_cache[fi.fileName()].first.isEmpty())
    {
        return QString("<img src=\"%1\" width=\"%2\">")
               .arg(m_cache[fi.fileName()].first)
               .arg(Settings::get()->GUIPreviewWidth);
    }
    return tr("No thumbnail");
}



#if 0

MetadataThumbnailMap MetadataCache::partThumbnailPaths(const QString &path)
{
    if (!m_map.contains(path))
        load(path);
    return m_map[path]->partThumbnailPaths();
}

MetadataThumbnailMap Metadata::partThumbnailPaths()
{
//return m_thumbnailsCache;
    if (m_thumbnailsCache.size())
        return m_thumbnailsCache;

    QDir d(m_path + "/" + THUMBNAILS_DIR);
    QStringList thumbs = d.entryList(QStringList() << "*.png" << "*.jpg",
                                     QDir::Files | QDir::Readable);

    QFileInfo fi;
    // includes first because local dir overrides it
    foreach(Metadata *include, includes)
    {
        MetadataThumbnailMapIterator it(include->partThumbnailPaths());
        while (it.hasNext())
        {
            it.next();
            fi.setFile(it.key());
            m_thumbnailsCache[fi.baseName()] = it.value();
        }
    }

    QString pmPath;
    foreach (QString i, thumbs)
    {
        fi.setFile(i);
        pmPath = m_path + "/" + THUMBNAILS_DIR + "/" + i;
        m_thumbnailsCache[fi.baseName()] = qMakePair(pmPath, QPixmap(pmPath));
    }

    return m_thumbnailsCache;
}
# endif
