#include "thumbnailmanager.h"
#include "settings.h"
#include <QtDebug>

ThumbnailWorker::ThumbnailWorker(const QString &path)
{
    m_path = path;
}

void ThumbnailWorker::run()
{
    ThumbnailMap ret;
    Metadata* m = MetadataCache::get()->metadata(m_path);
    getThumbs(m, &ret);
    emit dataReady(ret);
}

void ThumbnailWorker::cacheThumbnails(const QString &dirpath, ThumbnailMap* map)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (isInterruptionRequested())
        return;
#endif

    QString pmPath;
    QFileInfo fi;
    QDir d(dirpath);

    foreach(QString i, d.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files | QDir::Readable))
    {
        if (isInterruptionRequested())
            return;
        fi.setFile(i);
        if (map->contains(fi.baseName()))
            continue;
        pmPath = dirpath + "/" + i;
        map->insert(fi.baseName(), qMakePair(pmPath, QPixmap(pmPath).scaled(Settings::get()->GUIThumbWidth,
                                                                            Settings::get()->GUIThumbWidth,
                                                                            Qt::KeepAspectRatio)));
    };
}

void ThumbnailWorker::getThumbs(Metadata *m, ThumbnailMap* map)
{
    // local pictures
    cacheThumbnails(m->path(), map);
    // index
    cacheThumbnails(m->path() + "/" + THUMBNAILS_DIR, map);
    // now includes
    foreach(Metadata *i, m->includes())
    {
        getThumbs(i, map);
    }
}


ThumbnailManager::ThumbnailManager(QObject *parent)
    : QObject(parent),
      m_worker(0),
      m_isLoading(true)
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
        disconnect(m_worker, SIGNAL(dataReady(ThumbnailMap)), this, SLOT(dataReady(ThumbnailMap)));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)) 
        m_worker->requestInterruption();
#endif
        m_worker->wait();
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
    m_isLoading = true;
    m_worker->start();
}

void ThumbnailManager::dataReady(const ThumbnailMap &data)
{
    m_isLoading = false;
    m_cache = data;
    emit updateModel();
}

QPixmap ThumbnailManager::thumbnail(const QFileInfo &fi)
{
    if (m_isLoading)
        return m_loading;
    else if (m_cache.contains(fi.baseName()) && !m_cache[fi.baseName()].second.isNull())
        return m_cache[fi.baseName()].second;
    else
        return QPixmap();
}

QString ThumbnailManager::tooltip(const QFileInfo &fi)
{
    if (!m_cache[fi.baseName()].first.isEmpty())
    {
        return QString("<img src=\"%1\" width=\"%2\">")
               .arg(m_cache[fi.baseName()].first)
               .arg(Settings::get()->GUIPreviewWidth);
    }
    return tr("No thumbnail");
}

QString ThumbnailManager::path(const QFileInfo &fi)
{
    if (!m_cache[fi.fileName()].first.isEmpty())
    {
        return m_cache[fi.fileName()].first;
    }
    return QString();
}
