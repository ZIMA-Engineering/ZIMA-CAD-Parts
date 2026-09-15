#ifndef PARTSREAD_H
#define PARTSREAD_H
#include <QFileInfoList>
#include <QHash>
#include <QSettings>
namespace PartsCore {
QString partBaseName(const QFileInfo &fileInfo);
QFileInfoList listEntries(const QString &directory);
QString includePath(const QString &directory, const QString &raw);
struct ParameterValue { QString value; bool final = false; };
ParameterValue localParameter(QSettings &settings, const QString &directory,
    const QString &partName, const QString &param, const QString &language,
    QHash<QString, QStringList> &legacyGroups, bool &legacyLoaded);
}
#endif
