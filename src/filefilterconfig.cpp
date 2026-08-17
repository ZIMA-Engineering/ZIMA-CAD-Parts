#include "filefilterconfig.h"

#include "settings.h"

#include <QDir>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QTextStream>

namespace {

QStringList splitList(const QVariant &value)
{
    QStringList values;
    if (value.metaType().id() == QMetaType::QStringList)
        values = value.toStringList();
    else
        values = value.toString().split(QRegularExpression("[;,]"), Qt::SkipEmptyParts);
    for (QString &item : values)
    {
        item = item.trimmed();
        // Migrate the incorrect NX assembly pattern written by older releases.
        if (item.compare("*.asm_.asm", Qt::CaseInsensitive) == 0)
            item = "*.asm_.prt";
    }
    values.removeAll(QString());
    return values;
}

int groupIndex(const QVector<FileFilterGroupDefinition> &groups, const QString &id)
{
    for (int i = 0; i < groups.size(); ++i)
    {
        if (groups.at(i).id.compare(id, Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}

bool wildcardMatches(const QString &pattern, const QString &fileName)
{
    const QRegularExpression expression(
                QRegularExpression::wildcardToRegularExpression(pattern.trimmed()),
                QRegularExpression::CaseInsensitiveOption);
    return expression.match(fileName).hasMatch();
}

}

FileFilterConfig FileFilterConfig::load(const QString &directory)
{
    FileFilterConfig config;
    const QString root = dataSourceRoot(directory);
    if (root.isEmpty())
        return config;

    QDir rootDir(root);
    const QString relative = rootDir.relativeFilePath(QDir::cleanPath(directory));
    QStringList directories;
    directories << root;

    if (relative != "." && !relative.startsWith(".."))
    {
        QString current = root;
        const QStringList components = relative.split('/', Qt::SkipEmptyParts);
        for (const QString &component : components)
        {
            current = QDir(current).filePath(component);
            directories << current;
        }
    }

    for (const QString &path : directories)
    {
        const QString iniPath = configPath(path);
        if (!QFileInfo::exists(iniPath))
            continue;

        QSettings settings(iniPath, QSettings::IniFormat);
        settings.beginGroup("files");
        const bool inherits = settings.value("inherit", true).toBool();
        settings.endGroup();
        if (!inherits)
        {
            config.groups.clear();
            config.sourceFiles.clear();
        }
        mergeFile(config, iniPath);
    }

    return config;
}

QString FileFilterConfig::dataSourceRoot(const QString &directory)
{
    const QString cleanDirectory = QDir::cleanPath(directory);
    QString bestMatch;
#ifdef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif

    for (DataSource *source : Settings::get()->DataSources)
    {
        const QString root = QDir::cleanPath(source->rootPath);
        if (cleanDirectory.compare(root, sensitivity) == 0
                || cleanDirectory.startsWith(root + QLatin1Char('/'), sensitivity))
        {
            if (root.size() > bestMatch.size())
                bestMatch = root;
        }
    }
    return bestMatch;
}

QString FileFilterConfig::configPath(const QString &directory)
{
    return QDir(directory).filePath("0000-index/files.ini");
}

QString FileFilterConfig::versionModeName(FileVersionMode mode)
{
    if (mode == FileVersionMode::ProE)
        return "proe";
    if (mode == FileVersionMode::ZimaCad)
        return "zima-cad";
    return "none";
}

FileVersionMode FileFilterConfig::versionModeFromName(const QString &name)
{
    if (name.compare("proe", Qt::CaseInsensitive) == 0)
        return FileVersionMode::ProE;
    if (name.compare("zima-cad", Qt::CaseInsensitive) == 0)
        return FileVersionMode::ZimaCad;
    return FileVersionMode::None;
}

void FileFilterConfig::mergeFile(FileFilterConfig &config, const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    const bool modernFormat = settings.childGroups().contains("files", Qt::CaseInsensitive)
            && settings.contains("files/schema");
    if (!modernFormat && settings.childGroups().contains("Files", Qt::CaseInsensitive))
    {
        const QStringList includePatterns = splitList(settings.value("Files/Include"));
        QStringList versionPatterns = splitList(settings.value("Files/IncludeVersionFiles"));
        for (QString &pattern : versionPatterns)
        {
            if (pattern.endsWith(".*"))
                pattern.chop(2);
        }

        FileFilterGroupDefinition group;
        group.id = "legacy";
        group.title = QObject::tr("Imported file rules");
        group.versionMode = versionPatterns.isEmpty() ? FileVersionMode::None : FileVersionMode::ProE;
        group.showVersions = settings.value("Files/VersionFiles", "latest").toString()
                .compare("latest", Qt::CaseInsensitive) != 0;
        QStringList patterns = includePatterns;
        patterns.append(versionPatterns);
        patterns.removeDuplicates();
        for (const QString &pattern : patterns)
            group.formats << FileFilterDefinition{pattern, true};

        if (!group.formats.isEmpty())
        {
            config.groups << group;
            config.configured = true;
            config.sourceFiles << path;
        }
        return;
    }

    settings.beginGroup("files");
    config.inherit = settings.value("inherit", true).toBool();
    const bool hasOrder = settings.contains("groups");
    const QStringList order = splitList(settings.value("groups"));
    settings.endGroup();

    settings.beginGroup("group");
    const QStringList ids = settings.childGroups();
    for (const QString &id : ids)
    {
        settings.beginGroup(id);
        int index = groupIndex(config.groups, id);
        if (index < 0)
        {
            FileFilterGroupDefinition group;
            group.id = id;
            group.title = id;
            config.groups << group;
            index = config.groups.size() - 1;
        }

        FileFilterGroupDefinition &group = config.groups[index];
        if (settings.contains("title"))
            group.title = settings.value("title").toString().trimmed();
        if (settings.contains("enabled"))
            group.enabled = settings.value("enabled").toBool();
        if (settings.contains("patterns"))
        {
            group.formats.clear();
            for (const QString &pattern : splitList(settings.value("patterns")))
                group.formats << FileFilterDefinition{pattern, true};
        }
        const QStringList disabledPatterns = splitList(settings.value("disabledPatterns"));
        for (FileFilterDefinition &format : group.formats)
            format.enabled = !disabledPatterns.contains(format.pattern, Qt::CaseInsensitive);
        if (settings.contains("versionMode"))
            group.versionMode = versionModeFromName(settings.value("versionMode").toString());
        if (settings.contains("showVersions"))
            group.showVersions = settings.value("showVersions").toBool();
        settings.endGroup();
    }
    settings.endGroup();

    if (hasOrder)
    {
        QVector<FileFilterGroupDefinition> ordered;
        for (const QString &id : order)
        {
            const int index = groupIndex(config.groups, id);
            if (index >= 0)
                ordered << config.groups.at(index);
        }
        for (const FileFilterGroupDefinition &group : config.groups)
        {
            if (groupIndex(ordered, group.id) < 0)
                ordered << group;
        }
        config.groups = ordered;
    }

    config.configured = true;
    config.sourceFiles << path;
}

bool FileFilterConfig::save(const QString &directory, QString *errorMessage) const
{
    const QString metadataDirectory = QDir(directory).filePath("0000-index");
    if (!QDir().mkpath(metadataDirectory))
    {
        if (errorMessage)
            *errorMessage = QObject::tr("Unable to create %1.").arg(metadataDirectory);
        return false;
    }

    QSaveFile file(configPath(directory));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    QTextStream stream(&file);
    stream << "[files]\n";
    stream << "schema=1\n";
    stream << "inherit=" << (inherit ? "true" : "false") << "\n";
    QStringList ids;
    for (const FileFilterGroupDefinition &group : groups)
        ids << group.id;
    stream << "groups=" << ids.join(',') << "\n\n";

    for (const FileFilterGroupDefinition &group : groups)
    {
        stream << "[group/" << group.id << "]\n";
        stream << "title=" << group.title << "\n";
        QStringList patterns;
        QStringList disabledPatterns;
        for (const FileFilterDefinition &format : group.formats)
        {
            patterns << format.pattern;
            if (!format.enabled)
                disabledPatterns << format.pattern;
        }
        stream << "patterns=" << patterns.join(',') << "\n";
        if (!disabledPatterns.isEmpty())
            stream << "disabledPatterns=" << disabledPatterns.join(',') << "\n";
        stream << "enabled=" << (group.enabled ? "true" : "false") << "\n";
        stream << "versionMode=" << versionModeName(group.versionMode) << "\n";
        stream << "showVersions=" << (group.showVersions ? "true" : "false") << "\n\n";
    }

    if (!file.commit())
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }
    return true;
}

FileFilterMatch FileFilterConfig::match(const QString &fileName) const
{
    FileFilterMatch result;
    if (!configured)
    {
        result.accepted = true;
        return result;
    }

    static const QRegularExpression proeExpression(
                "^(.+)\\.(prt|asm|drw|frm|neu)\\.(\\d+)$",
                QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression zimaExpression(
                "^(.+)\\.(prtz|asmz|drwz)(?:\\.(\\d+))?$",
                QRegularExpression::CaseInsensitiveOption);

    for (const FileFilterGroupDefinition &group : groups)
    {
        if (!group.enabled)
            continue;

        QString logicalFileName = fileName;
        QRegularExpressionMatch versionMatch;
        if (group.versionMode == FileVersionMode::ProE)
            versionMatch = proeExpression.match(fileName);
        else if (group.versionMode == FileVersionMode::ZimaCad)
            versionMatch = zimaExpression.match(fileName);

        if (versionMatch.hasMatch())
        {
            logicalFileName = versionMatch.captured(1) + '.' + versionMatch.captured(2);
            result.logicalName = logicalFileName;
            if (!versionMatch.captured(3).isEmpty())
                result.version = versionMatch.captured(3).toInt();
        }

        for (const FileFilterDefinition &format : group.formats)
        {
            if (format.enabled && wildcardMatches(format.pattern, logicalFileName))
            {
                result.accepted = true;
                result.versionMode = group.versionMode;
                result.showVersions = group.showVersions;
                if (result.logicalName.isEmpty())
                    result.logicalName = logicalFileName;
                return result;
            }
        }
    }
    return result;
}
