#ifndef LOCALFILTERS_H
#define LOCALFILTERS_H

#include <QBitArray>
#include <QDir>
#include <QFileInfoList>
#include <QHash>
#include <QRegularExpression>
#include <QSettings>

// Deliberately local: no parent directories or metadata includes are consulted.
class LocalFilters
{
public:
    QStringList hidden;
    bool showVersions = true;
    bool showZimaVersions = true;

    static QString filePath(const QString &directory)
    {
        return QDir(directory).filePath("0000-index/filters.ini");
    }

    void load(const QString &directory, bool defaultShowVersions)
    {
        QSettings settings(filePath(directory), QSettings::IniFormat);
        settings.setFallbacksEnabled(false);
        hidden = settings.value("Filters/Hide").toStringList();
        showZimaVersions = settings.value("Filters/ShowZimaVersions", true).toBool();
        showVersions = settings.value("Filters/ShowVersions", defaultShowVersions).toBool();
    }

    QBitArray accepted(const QFileInfoList &files, bool showDirectories) const
    {
        QList<QRegularExpression> patterns;
        for (const QString &pattern : hidden) {
            if (!pattern.trimmed().isEmpty())
                patterns.append(QRegularExpression(
                    QRegularExpression::wildcardToRegularExpression(pattern.trimmed()),
                    QRegularExpression::CaseInsensitiveOption));
        }
        static const QRegularExpression versionRx(
            "^(.+\\.(?:prt|asm|drw|frm|neu))\\.([0-9]+)$",
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression zimaArchiveRx(
            "^.+\\.(?:prtz|asmz|drwz|frmz|tblz)\\.[0-9]+$",
            QRegularExpression::CaseInsensitiveOption);
        QHash<QString, QPair<qulonglong, int>> latest;
        QBitArray result(files.size(), true);
        for (int row = 0; row < files.size(); ++row) {
            const QFileInfo &file = files.at(row);
            const QString name = file.fileName();
            if (file.isDir() && (name == "0000-index" || !showDirectories)) {
                result.clearBit(row);
                continue;
            }
            for (const QRegularExpression &pattern : patterns) {
                if (pattern.match(name).hasMatch()) {
                    result.clearBit(row);
                    break;
                }
            }
            // ZIMA-CAD saves the current document without a number. Numbered
            // files are archives, not candidates for the current document.
            if (!showZimaVersions && !file.isDir() && zimaArchiveRx.match(name).hasMatch())
                result.clearBit(row);
            // Choose the newest version before exclusions: hiding it must not
            // silently make an older revision appear to be the current one.
            if (!showVersions && !file.isDir()) {
                const auto match = versionRx.match(name);
                if (!match.hasMatch())
                    continue;
                bool valid = false;
                const auto version = match.captured(2).toULongLong(&valid);
                if (!valid)
                    continue;
                const QString key = match.captured(1);
                auto previous = latest.find(key);
                if (previous == latest.end()) {
                    latest.insert(key, qMakePair(version, row));
                } else if (version > previous->first) {
                    result.clearBit(previous->second);
                    *previous = qMakePair(version, row);
                } else {
                    result.clearBit(row);
                }
            }
        }
        return result;
    }
};

#endif
