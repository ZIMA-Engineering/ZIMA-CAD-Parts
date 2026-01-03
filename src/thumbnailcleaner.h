#ifndef THUMBNAILCLEANER_H
#define THUMBNAILCLEANER_H

#include <QObject>
#include <QFileInfoList>
#include <QSet>

class ThumbnailCleaner : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailCleaner(QWidget *parent = nullptr);

    void cleanupUnusedThumbnails(const QString &directory);

private:
    QFileInfoList findUnusedThumbnails(const QString &directory) const;
    QSet<QString> collectBaseNames(const QString &directory) const;
};

#endif // THUMBNAILCLEANER_H
