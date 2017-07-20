#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include <QFileInfo>
#include <QPixmap>
#include <QThread>

#include "metadata.h"

//! \brief Thumbnail map: baseName -> full path to the file (including the file name)
typedef QHash<QString,QPair<QString,QPixmap> > ThumbnailMap;


class ThumbnailWorker : public QThread
{
    Q_OBJECT

public:
    ThumbnailWorker(const QString &path);

signals:
    void dataReady(const ThumbnailMap &data);

protected:
    void run();

private:
    QString m_path;

    void cacheThumbnails(const QString &dirpath, ThumbnailMap* map);
    void getThumbs(Metadata *m, ThumbnailMap* map);
};

class ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailManager(QObject *parent = 0);

    QPixmap thumbnail(const QFileInfo &fi);
    QString tooltip(const QFileInfo &fi);
    QString path(const QFileInfo &fi);

signals:
    void updateModel();

public slots:
    void setPath(const QString &path);
    void clear();
    void dataReady(const ThumbnailMap &data);

private:
    ThumbnailWorker *m_worker;

    QString m_path;
    QPixmap m_loading;
    ThumbnailMap m_cache;
    bool m_isLoading;

    void load();
};

#endif // THUMBNAILMANAGER_H
