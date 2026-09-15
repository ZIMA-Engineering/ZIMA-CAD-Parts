#ifndef PARTSQUERY_H
#define PARTSQUERY_H
#include "partsread.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QTemporaryDir>
#include <memory>
namespace PartsCore {
class DirectorySnapshot
{
public:
    explicit DirectorySnapshot(const QString &path, QSet<QString> ancestors = {});
    QStringList handles() const;
    QStringList labels(const QString &language) const;
    QString parameter(const QString &part, const QString &handle, const QString &language);
    bool showDirectories() const;
    QJsonObject part(const QFileInfo &file, const QString &language);
private:
    QString m_path;
    QTemporaryDir m_temporary;
    std::unique_ptr<QSettings> m_settings;
    QList<std::shared_ptr<DirectorySnapshot>> m_includes;
    QHash<QString, QStringList> m_legacy;
    bool m_legacyLoaded = false;
};
QJsonObject listParts(const QString &directory, const QString &language,
                      bool defaultShowVersions, const QString &nameFilter = QString());
}
#endif
