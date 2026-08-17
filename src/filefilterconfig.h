#ifndef FILEFILTERCONFIG_H
#define FILEFILTERCONFIG_H

#include <QFileInfo>
#include <QStringList>
#include <QVector>

enum class FileVersionMode
{
    None,
    ProE,
    ZimaCad
};

struct FileFilterDefinition
{
    QString pattern;
    bool enabled = true;
};

struct FileFilterGroupDefinition
{
    QString id;
    QString title;
    QVector<FileFilterDefinition> formats;
    FileVersionMode versionMode = FileVersionMode::None;
    bool showVersions = false;
    bool enabled = true;
};

struct FileFilterMatch
{
    bool accepted = false;
    FileVersionMode versionMode = FileVersionMode::None;
    bool showVersions = true;
    QString logicalName;
    int version = -1;
};

class FileFilterConfig
{
public:
    static FileFilterConfig load(const QString &directory);
    static QString dataSourceRoot(const QString &directory);
    static QString configPath(const QString &directory);
    static QString versionModeName(FileVersionMode mode);
    static FileVersionMode versionModeFromName(const QString &name);

    bool save(const QString &directory, QString *errorMessage = nullptr) const;
    FileFilterMatch match(const QString &fileName) const;

    bool configured = false;
    bool inherit = true;
    QStringList sourceFiles;
    QVector<FileFilterGroupDefinition> groups;

private:
    static void mergeFile(FileFilterConfig &config, const QString &path);
};

#endif // FILEFILTERCONFIG_H
