#include "filerenamer.h"
#include "settings.h"
#include "partcache.h"
#include "metadata.h"

#include <QDir>
#include <QFile>
#include <QDebug>

FileRenamer::FileRenamer(QObject *parent)
	: QObject(parent)
{

}

bool FileRenamer::rename(const QString &dir, const QFileInfo &file, QString newName)
{
	/* Files in the current directory */
	if (!renameFilesInDir(dir, file.baseName(), newName))
		return false;

	/* Thumbnails in index directory */
	if (!renameFilesInDir(dir + "/" + THUMBNAILS_DIR, file.baseName(), newName))
		return false;

	/* Metadata */
	MetadataCache::get()->renamePart(dir, file.baseName(), newName);

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

	foreach (const QFileInfo &fi, entries) {
		QString newPath = QString("%1/%2.%3")
							.arg(fi.absolutePath())
							.arg(newName)
							.arg(fi.completeSuffix());

		qDebug() << "Rename" << fi.absoluteFilePath() << "to" << newPath;

		if (QFile::rename(fi.absoluteFilePath(), newPath)) {
			emit renamed(fi, QFileInfo(newPath));
		} else {
			emit error(fi);
			return false;
		}
	}

	return true;
}
