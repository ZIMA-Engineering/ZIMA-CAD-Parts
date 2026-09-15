#include "partsread.h"
#include <QDir>
#include <QRegularExpression>

QString PartsCore::partBaseName(const QFileInfo &fileInfo)
{
    if (fileInfo.isDir())
        return fileInfo.fileName();

    static const QRegularExpression cadName(
                "^(.+)\\.(?:prt|asm|drw|frm|neu|prtz|asmz|drwz|frmz|tblz)(?:\\.\\d+)?$",
                QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = cadName.match(fileInfo.fileName());
    if (match.hasMatch())
        return match.captured(1);

    return fileInfo.completeBaseName();
}

QFileInfoList PartsCore::listEntries(const QString &directory)
{
    return QDir(directory).entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden
        | QDir::System | QDir::NoDotAndDotDot, QDir::Name);
}

QString PartsCore::includePath(const QString &directory, const QString &raw)
{
    return QDir::cleanPath(QDir::isAbsolutePath(raw) ? raw : QDir(directory).filePath(raw));
}

PartsCore::ParameterValue PartsCore::localParameter(QSettings &settings,
    const QString &directory, const QString &partName, const QString &param,
    const QString &language, QHash<QString, QStringList> &legacyGroups, bool &legacyLoaded)
{
    const auto readValue = [&](const QString &partGroup)
    {
        QString anyValue;
        QString localizedValue;
        settings.beginGroup("Parts");
        settings.beginGroup(partGroup);
        settings.beginGroup(param);
        {
            foreach (const QString &lang, settings.childKeys())
            {
                const QString value = settings.value(lang).toString();

                if (!value.isEmpty() && lang == language)
                {
                    localizedValue = value;
                    break;
                }

                if (anyValue.isEmpty())
                    anyValue = value;
            }
        }
        settings.endGroup();
        settings.endGroup();
        settings.endGroup();

        if (!localizedValue.isEmpty())
            return localizedValue;
        if (!anyValue.isEmpty())
            return anyValue;
        return settings.value(QString("Parts/%1/%2").arg(partGroup, param)).toString();
    };

    QString ret = readValue(partName);
    // An explicitly cleared value must not resurrect a legacy value.
    const bool explicitlyStored = settings.contains(
        QString("Parts/%1/%2/%3").arg(partName, param, language))
        || settings.contains(QString("Parts/%1/%2").arg(partName, param));
    if (ret.isEmpty() && explicitlyStored)
        return {ret, true};
    if (ret.isEmpty()) {
        if (!legacyLoaded) {
            settings.beginGroup("Parts");
            const auto groups = settings.childGroups();
            settings.endGroup();
            for (const auto &group : groups) {
                const QFileInfo file(QDir(directory).filePath(group));
                if (file.isFile()) {
                    const auto base = PartsCore::partBaseName(file);
                    if (base != group)
                        legacyGroups[base].append(group);
                }
            }
            legacyLoaded = true;
        }
        for (const auto &group : legacyGroups.value(partName)) {
            ret = readValue(group);
            if (!ret.isEmpty())
                break;
        }
    }
    // Older releases stored dotted names under the text before the first dot.
    if (ret.isEmpty() && partName.contains('.'))
        ret = readValue(partName.section('.', 0, 0));

    return {ret, !ret.isEmpty()};
}
