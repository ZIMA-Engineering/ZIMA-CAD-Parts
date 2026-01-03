#include "thumbnailcleaner.h"

#include <QDir>
#include <QtDebug>

#include "settings.h"
#include "directoryremover.h"
#include "unusedthumbnailsdialog.h"

ThumbnailCleaner::ThumbnailCleaner(QWidget *parent)
    : QObject(parent)
{
}

void ThumbnailCleaner::cleanupUnusedThumbnails(const QString &directory)
{
    qDebug() << "ThumbnailCleaner: cleanup requested for" << directory;

    if (directory.isEmpty())
    {
        qDebug() << "ThumbnailCleaner: aborting cleanup, directory is empty";
        return;
    }

    QFileInfoList unused = findUnusedThumbnails(directory);

    if (unused.isEmpty())
    {
        qDebug() << "ThumbnailCleaner: no unused thumbnails found in" << directory;
        return;
    }

    qDebug() << "ThumbnailCleaner: unused thumbnails found" << unused.size();

    UnusedThumbnailsDialog dlg(directory, unused, static_cast<QWidget*>(parent()));

    if (dlg.exec() != QDialog::Accepted)
    {
        qDebug() << "ThumbnailCleaner: user declined deletion";
        return;
    }

    DirectoryRemover rm(static_cast<QWidget*>(parent()));
    rm.addFiles(unused);
    rm.setMessage(tr("Please wait while the unused thumbnails are being removed..."));
    rm.setStopOnError(false);
    rm.work();
}

QFileInfoList ThumbnailCleaner::findUnusedThumbnails(const QString &directory) const
{
    QFileInfoList unused;
    QDir thumbDir(directory + "/" + THUMBNAILS_DIR);

    if (!thumbDir.exists())
    {
        qDebug() << "ThumbnailCleaner: thumbnail dir does not exist at" << thumbDir.absolutePath();
        return unused;
    }

    QSet<QString> baseNames = collectBaseNames(directory);
    qDebug() << "ThumbnailCleaner: collected base names" << baseNames.size();
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";

    foreach (const QString &thumb, thumbDir.entryList(filters, QDir::Files | QDir::Readable))
    {
        QFileInfo fi(thumbDir.absoluteFilePath(thumb));

        if (!baseNames.contains(fi.baseName()))
        {
            qDebug() << "ThumbnailCleaner: thumbnail without match" << fi.absoluteFilePath();
            unused << fi;
        }
    }

    qDebug() << "ThumbnailCleaner: unused thumbnails count" << unused.size();
    return unused;
}

QSet<QString> ThumbnailCleaner::collectBaseNames(const QString &directory) const
{
    QSet<QString> baseNames;
    QDir dir(directory);

    QFileInfoList entries = dir.entryInfoList(
                                QDir::NoDotAndDotDot
                                | QDir::AllEntries
                                | QDir::Hidden
                                | QDir::System
                                | QDir::Readable
                            );

    foreach (const QFileInfo &fi, entries)
    {
        if (fi.fileName() == METADATA_DIR)
            continue;

        baseNames.insert(fi.baseName());
    }

    qDebug() << "ThumbnailCleaner: collected base names list size" << baseNames.size();
    return baseNames;
}
