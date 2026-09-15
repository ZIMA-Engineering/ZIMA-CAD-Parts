#ifndef DIRECTORYPROTECTION_H
#define DIRECTORYPROTECTION_H

#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>
#include <functional>

class DirectoryProtection
{
    Q_DECLARE_TR_FUNCTIONS(DirectoryProtection)
public:
    struct ApplyResult {
        int updated = 0;
        QStringList failed;
        bool canceled = false;
    };
    static ApplyResult applyToSubdirectories(const QString &directory, bool locked,
        const std::function<bool(const QString &)> &continueWork = {});
    static bool isLocked(const QString &directory);
    // Local file protection; whole-folder operations also inspect contained locks.
    static QString removalLock(const QFileInfo &entry);
    static QString message(const QString &lockedDirectory);
};

#endif
