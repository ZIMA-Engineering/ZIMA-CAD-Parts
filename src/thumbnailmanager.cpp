#include "thumbnailmanager.h"
#include "file.h"
#include <QDir>
#include <QImageReader>
#include <QMutexLocker>
#include <QSettings>

namespace {
bool isImage(const QFileInfo &file)
{
    const QString suffix = file.suffix().toLower();
    return suffix == "png" || suffix == "jpg" || suffix == "jpeg";
}
}

ThumbnailWorker::ThumbnailWorker(QObject *parent) : QThread(parent) {}
ThumbnailWorker::~ThumbnailWorker()
{
    {
        QMutexLocker lock(&m_mutex);
        m_stopping = true;
        ++m_generation;
        m_queue.clear();
        m_workAvailable.wakeOne();
    }
    // Only destruction waits; navigation never waits for an image decoder.
    wait();
}
void ThumbnailWorker::setContext(quint64 generation, const QString &directory, int width)
{
    QMutexLocker lock(&m_mutex);
    m_generation = generation;
    m_directory = directory;
    m_width = width;
    m_queue.clear();
    m_workAvailable.wakeOne();
}
void ThumbnailWorker::enqueue(quint64 generation, const QFileInfo &file)
{
    QMutexLocker lock(&m_mutex);
    if (generation != m_generation || m_stopping)
        return;
    m_queue.enqueue(file);
    m_workAvailable.wakeOne();
}
ThumbnailPaths ThumbnailWorker::discover(const QString &directory, const std::function<bool()> &cancelled)
{
    ThumbnailPaths result;
    QSet<QString> visited;
    std::function<void(const QString &)> visit = [&](const QString &path) {
        if (cancelled())
            return;
        QFileInfo info(path);
        QString identity = info.canonicalFilePath();
        if (identity.isEmpty())
            identity = info.absoluteFilePath();
        if (visited.contains(identity))
            return;
        visited.insert(identity);
        for (const QString &folder : {path, QDir(path).filePath("0000-index/thumbnails")}) {
            // Enumerate names only; decode images only for requested rows.
            const auto files = QDir(folder).entryInfoList({"*.png", "*.jpg", "*.jpeg"},
                QDir::Files | QDir::Readable | QDir::Hidden, QDir::Name);
            for (const QFileInfo &file : files) {
                if (cancelled())
                    return;
                if (!result.contains(File::partBaseName(file)))
                    result.insert(File::partBaseName(file), file.absoluteFilePath());
            }
        }
        // Own settings instance: never touch the GUI's MetadataCache in a worker.
        QSettings metadata(QDir(path).filePath("0000-index/metadata.ini"), QSettings::IniFormat);
        metadata.setFallbacksEnabled(false);
        for (const QString &include : metadata.value("Directory/IncludeThumbnails").toStringList()) {
            if (!include.isEmpty())
                visit(QDir::cleanPath(QDir(path).absoluteFilePath(include)));
        }
    };
    if (!directory.isEmpty())
        visit(directory);
    return result;
}
void ThumbnailWorker::run()
{
    quint64 indexedGeneration = 0;
    ThumbnailPaths sources;
    for (;;) {
        QFileInfo file;
        QString directory;
        quint64 generation;
        int width;
        {
            QMutexLocker lock(&m_mutex);
            while (m_queue.isEmpty() && !m_stopping)
                m_workAvailable.wait(&m_mutex);
            if (m_stopping)
                return;
            generation = m_generation;
            directory = m_directory;
            width = m_width;
            file = m_queue.dequeue();
        }
        const auto cancelled = [this, generation] { return m_generation != generation; };
        QString source;
        if (!file.isDir() && isImage(file)) {
            source = file.absoluteFilePath();
        } else {
            if (indexedGeneration != generation) {
                sources = discover(directory, cancelled);
                if (cancelled())
                    continue;
                indexedGeneration = generation;
                emit sourcesReady(generation, sources);
            }
            source = sources.value(File::partBaseName(file));
        }
        if (cancelled())
            continue;
        QImage image;
        if (!source.isEmpty()) {
            QImageReader reader(source);
            reader.setAutoTransform(true);
            const QSize size = reader.size();
            if (size.isValid())
                reader.setScaledSize(size.scaled(width, width, Qt::KeepAspectRatio));
            image = reader.read();
            if (!image.isNull())
                image = image.scaled(width, width, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        if (!cancelled())
            emit imageReady(generation, file.absoluteFilePath(), image);
    }
}
ThumbnailManager::ThumbnailManager(QObject *parent)
    : QObject(parent), m_worker(new ThumbnailWorker(this)),
      m_loading(":/gfx/image-loading.png"), m_cache(32 * 1024 * 1024)
{
    qRegisterMetaType<ThumbnailPaths>("ThumbnailPaths");
    connect(m_worker, &ThumbnailWorker::sourcesReady, this,
        [this](quint64 generation, const ThumbnailPaths &sources) {
            if (generation != m_generation)
                return;
            m_sources = sources;
            m_sourcesLoaded = true;
        });
    connect(m_worker, &ThumbnailWorker::imageReady, this,
        [this](quint64 generation, const QString &file, const QImage &image) {
            if (generation != m_generation)
                return;
            m_pending.remove(file);
            if (image.isNull()) {
                m_missing.insert(file);
            } else {
                auto pixmap = new QPixmap(QPixmap::fromImage(image));
                m_cache.insert(file, pixmap, qMax(1, int(image.sizeInBytes())));
            }
            emit thumbnailReady(file);
        });
    m_worker->start();
}
void ThumbnailManager::setPath(const QString &path, int width)
{
    m_path = path;
    m_width = qBound(1, width, 2048);
    clear();
}
void ThumbnailManager::clear()
{
    cancelPending();
    m_cache.clear();
    m_missing.clear();
    m_sources.clear();
    m_sourcesLoaded = false;
}
void ThumbnailManager::cancelPending()
{
    m_worker->setContext(++m_generation, m_path, m_width);
    m_pending.clear();
}

QPixmap ThumbnailManager::thumbnail(const QFileInfo &file)
{
    const QString key = file.absoluteFilePath();
    if (const auto cached = m_cache.object(key))
        return *cached;
    if (m_missing.contains(key) || m_path.isEmpty())
        return QPixmap();
    if (!m_pending.contains(key)) {
        m_pending.insert(key);
        m_worker->enqueue(m_generation, file);
    }
    return m_loading;
}
QString ThumbnailManager::tooltip(const QFileInfo &file, int width) const
{
    const QString source = isImage(file) && !file.isDir()
        ? file.absoluteFilePath() : m_sources.value(File::partBaseName(file));
    if (source.isEmpty())
        return tr("No thumbnail");
    return QString("<img src=\"%1\" width=\"%2\">").arg(source.toHtmlEscaped()).arg(width);
}
QString ThumbnailManager::path(const QFileInfo &file)
{
    // Explicit file operations must work even before any previews have loaded.
    if (!m_sourcesLoaded) {
        m_sources = ThumbnailWorker::discover(m_path, [] { return false; });
        m_sourcesLoaded = true;
    }
    return m_sources.value(File::partBaseName(file));
}
