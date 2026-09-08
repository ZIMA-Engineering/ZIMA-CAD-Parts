#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H
#include <QCache>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QMutex>
#include <QPixmap>
#include <QQueue>
#include <QSet>
#include <QThread>
#include <QWaitCondition>
#include <atomic>
#include <functional>

using ThumbnailPaths = QHash<QString, QString>;

class ThumbnailWorker : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailWorker(QObject *parent = nullptr);
    ~ThumbnailWorker() override;
    void setContext(quint64 generation, const QString &directory, int width);
    void enqueue(quint64 generation, const QFileInfo &file);
    static ThumbnailPaths discover(const QString &directory, const std::function<bool()> &cancelled);
signals:
    void sourcesReady(quint64 generation, const ThumbnailPaths &paths);
    void imageReady(quint64 generation, const QString &file, const QImage &image);
protected:
    void run() override;
private:
    QMutex m_mutex;
    QWaitCondition m_workAvailable;
    QQueue<QFileInfo> m_queue;
    QString m_directory;
    int m_width = 32;
    std::atomic<quint64> m_generation{0};
    bool m_stopping = false;
};

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = nullptr);
    QPixmap thumbnail(const QFileInfo &file);
    QString tooltip(const QFileInfo &file, int width = 256) const;
    QString path(const QFileInfo &file);
    void setPath(const QString &path, int width = 32);
    void clear();
    void cancelPending();
signals:
    void thumbnailReady(const QString &file);
private:
    ThumbnailWorker *m_worker;
    QString m_path;
    int m_width = 32;
    quint64 m_generation = 0;
    QPixmap m_loading;
    QCache<QString, QPixmap> m_cache;
    QSet<QString> m_pending;
    QSet<QString> m_missing;
    ThumbnailPaths m_sources;
    bool m_sourcesLoaded = false;
};
#endif
