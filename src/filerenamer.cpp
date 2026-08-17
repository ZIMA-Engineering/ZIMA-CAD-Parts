#include "filerenamer.h"
#include "settings.h"
#include "partcache.h"
#include "metadata.h"
#include "file.h"

#include <QDir>
#include <QFile>
#include <QDebug>

FileRenamer::FileRenamer(QObject *parent)
    : QObject(parent)
{

}

bool FileRenamer::rename(const QString &dir, const QFileInfo &file, QString newName)
{
    newName = newName.trimmed();
    if (!File::isValidPartName(newName))
        return false;

    bool isDir = file.isDir();
    QString newDirPath;

    if (isDir) {
        /* Rename part directory */
        newDirPath = file.absolutePath() +"/"+ newName;

        qDebug() << "Rename" << file.absoluteFilePath() << "to" << newDirPath;

        if (!QFile::rename(file.absoluteFilePath(), newDirPath))
            return false;
    } else {
        /* Files in the current directory */
        if (!renameFilesInDir(dir, File::partBaseName(file), newName))
            return false;
    }

    /* Thumbnails in index directory */
    if (!renameFilesInDir(dir + "/" + THUMBNAILS_DIR, File::partBaseName(file), newName))
        return false;

    /* Metadata */
    MetadataCache::get()->renamePart(dir, File::partBaseName(file), newName);

    /* Directory rename */
    if (isDir) {
        PartCache::get()->renameDirectory(file.absoluteFilePath(), newDirPath);
        MetadataCache::get()->clearBelow(file.absoluteFilePath());
    }

    /* Refresh models/views */
    PartCache::get()->refresh(dir);

    return true;
}

bool FileRenamer::renameFilesInDir(const QString &dir, const QString &oldName, const QString &newName)
{
    QDir d(dir);
    QFileInfoList entries = d.entryInfoList(
                                (QStringList() << QString("%1.*").arg(oldName)),
                                QDir::Files
                            );
    QFile f;

    foreach (const QFileInfo &fi, entries) {
        const QString suffix = fi.fileName().mid(oldName.size());
        QString newPath = QString("%1/%2%3")
                          .arg(fi.absolutePath())
                          .arg(newName)
                          .arg(suffix);

        qDebug() << "Rename" << fi.absoluteFilePath() << "to" << newPath;
        f.setFileName(fi.absoluteFilePath());

        if (f.rename(newPath)) {
            emit renamed(fi, QFileInfo(newPath));
        } else {
            emit error(fi, QFileInfo(newPath), f.errorString());
            return false;
        }
    }

    return true;
}
