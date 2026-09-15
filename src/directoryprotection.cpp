#include "directoryprotection.h"
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include "metadata.h"

bool DirectoryProtection::isLocked(const QString &directory)
{
    const QString path = QDir(directory).filePath("0000-index/metadata.ini");
    if (!QFileInfo::exists(path))
        return false;
    QSettings settings(path, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    return settings.value("Directory/PreventRemoval", false).toBool();
}

QString DirectoryProtection::removalLock(const QFileInfo &entry)
{
    const QString parent = entry.absolutePath();
    if (!entry.isDir() || entry.isSymLink())
        return isLocked(parent) ? parent : QString();

    const QString directory = entry.absoluteFilePath();
    if (isLocked(directory))
        return directory;
    // Removing an index folder would remove the switch itself.
    if (entry.fileName() == "0000-index" && isLocked(parent))
        return parent;
    QDirIterator children(directory, QDir::Dirs | QDir::NoDotAndDotDot
                          | QDir::Hidden | QDir::System | QDir::NoSymLinks,
                          QDirIterator::Subdirectories);
    while (children.hasNext()) {
        const QString child = children.next();
        if (isLocked(child))
            return child;
    }
    return QString();
}

QString DirectoryProtection::message(const QString &lockedDirectory)
{
    return tr("Directory '%1' is locked. Unlock it in Directory properties before deleting or moving its files.")
            .arg(lockedDirectory);
}

DirectoryProtection::ApplyResult DirectoryProtection::applyToSubdirectories(
        const QString &directory, bool locked,
        const std::function<bool(const QString &)> &continueWork)
{
    ApplyResult result;
    QStringList pending{directory};
    while (!pending.isEmpty()) {
        const QString parent = pending.takeLast();
        if (continueWork && !continueWork(parent)) {
            result.canceled = true;
            break;
        }
        const QFileInfo parentInfo(parent);
        if (!parentInfo.isDir() || !parentInfo.isReadable()) {
            result.failed.append(parent);
            continue;
        }
        const auto children = QDir(parent).entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System | QDir::NoSymLinks);
        for (const auto &child : children) {
            if (child.fileName() == "0000-index")
                continue;
            const QString path = child.absoluteFilePath();
            if (continueWork && !continueWork(path)) {
                result.canceled = true;
                return result;
            }
            pending.append(path);
            const QString index = QDir(path).filePath("0000-index");
            const QString file = QDir(index).filePath("metadata.ini");
            if (QFileInfo(index).isSymLink() || QFileInfo(file).isSymLink()
                    || !QDir().mkpath(index)) {
                result.failed.append(path);
                continue;
            }
            QSettings settings(file, QSettings::IniFormat);
            settings.setFallbacksEnabled(false);
            settings.setValue("Directory/PreventRemoval", locked);
            settings.sync();
            if (settings.status() != QSettings::NoError) {
                result.failed.append(path);
                continue;
            }
            ++result.updated;
            emit MetadataCache::get()->removalLockChanged(path);
        }
    }
    return result;
}
