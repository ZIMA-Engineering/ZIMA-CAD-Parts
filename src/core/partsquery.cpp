#include "partsquery.h"
#include "../localfilters.h"
#include "../metadata/migrations/metadatav2migration.h"
#include <QDir>
#include <QThread>
#include <QFile>

PartsCore::DirectorySnapshot::DirectorySnapshot(const QString &path, QSet<QString> ancestors)
    : m_path(QFileInfo(path).absoluteFilePath())
{
    if (QThread::currentThread()->isInterruptionRequested()) throw QString("Cancelled");
    QFileInfo directory(m_path);
    if (!directory.isDir() || !directory.isReadable())
        throw QString("Directory is missing or unreadable: %1").arg(m_path);
    QString identity = directory.canonicalFilePath();
#ifdef Q_OS_WIN
    identity = identity.toLower();
#endif
    if (ancestors.contains(identity) || ancestors.size() >= 64)
        throw QString("Cyclic or excessively deep metadata includes: %1").arg(m_path);
    ancestors.insert(identity);
    const QString filename = QDir(m_path).filePath("0000-index/metadata.ini");
    if (QFileInfo::exists(filename)) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            throw QString("Cannot read metadata: %1").arg(filename);
    }
    m_settings = std::make_unique<QSettings>(filename, QSettings::IniFormat);
    m_settings->setFallbacksEnabled(false);
    const int format = m_settings->value("Directory/Version", 1).toInt();
    if (m_settings->status() != QSettings::NoError)
        throw QString("Invalid metadata: %1").arg(filename);
    if (format > 2 || format < 1)
        throw QString("Unsupported metadata version: %1").arg(format);
    if (format == 1 && !m_settings->allKeys().isEmpty()) {
        if (!m_temporary.isValid())
            throw QString("Cannot create temporary metadata snapshot");
        auto converted = std::make_unique<QSettings>(m_temporary.filePath("metadata.ini"), QSettings::IniFormat);
        for (const auto &key : m_settings->allKeys())
            converted->setValue(key, m_settings->value(key));
        MetadataV2Migration migration;
        migration.setSettings(converted.get());
        if (!migration.migrate())
            throw QString("Cannot read legacy metadata: %1").arg(filename);
        m_settings = std::move(converted);
    }
    auto includes = m_settings->value("Directory/IncludeParameters").toStringList();
    QStringList resolved;
    for (const auto &include : includes)
        resolved.append(includePath(m_path, include));
    resolved.removeDuplicates();
    for (const auto &include : resolved)
        m_includes.append(std::make_shared<DirectorySnapshot>(include, ancestors));
}

QStringList PartsCore::DirectorySnapshot::handles() const
{
    if (m_includes.isEmpty())
        return m_settings->value("Directory/Parameters").toStringList();
    QStringList result;
    for (const auto &include : m_includes)
        result.append(include->handles());
    return result;
}

QStringList PartsCore::DirectorySnapshot::labels(const QString &language) const
{
    QStringList result;
    if (m_includes.isEmpty()) {
        for (const auto &handle : handles())
            result.append(m_settings->value(QString("Parameters/%1/Label/%2").arg(handle, language)).toString());
    } else {
        for (const auto &include : m_includes)
            result.append(include->labels(language));
    }
    return result;
}

QString PartsCore::DirectorySnapshot::parameter(const QString &part, const QString &handle, const QString &language)
{
    const auto local = localParameter(*m_settings, m_path, part, handle, language, m_legacy, m_legacyLoaded);
    if (local.final)
        return local.value;
    for (const auto &include : m_includes) {
        const auto value = include->parameter(part, handle, language);
        if (!value.isEmpty())
            return value;
    }
    return QString();
}

bool PartsCore::DirectorySnapshot::showDirectories() const
{
    return m_settings->value("Directory/SubdirectoriesAsParts", false).toBool();
}

QJsonObject PartsCore::DirectorySnapshot::part(const QFileInfo &file, const QString &language)
{
    const auto base = partBaseName(file);
    QJsonObject values;
    for (const auto &handle : handles())
        values.insert(handle, parameter(base, handle, language));
    return {{"name", file.fileName()}, {"path", file.absoluteFilePath()},
            {"baseName", base}, {"directory", file.isDir()}, {"parameters", values}};
}

QJsonObject PartsCore::listParts(const QString &directory, const QString &language,
                                bool defaultShowVersions, const QString &nameFilter)
{
    DirectorySnapshot metadata(directory);
    const auto files = listEntries(directory);
    LocalFilters filters;
    filters.load(directory, defaultShowVersions);
    const auto accepted = filters.accepted(files, metadata.showDirectories());
    QJsonArray parts;
    for (int i = 0; i < files.size(); ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) throw QString("Cancelled");
        if (accepted.testBit(i) && files[i].fileName().contains(nameFilter, Qt::CaseInsensitive))
            parts.append(metadata.part(files[i], language));
    }
    QJsonArray columns;
    const auto handles = metadata.handles();
    const auto labels = metadata.labels(language);
    for (int i = 0; i < handles.size(); ++i)
        columns.append(QJsonObject{{"handle", handles[i]}, {"label", labels.value(i)}});
    return {{"schemaVersion", 1}, {"directory", QFileInfo(directory).absoluteFilePath()},
            {"language", language}, {"columns", columns}, {"parts", parts}};
}
