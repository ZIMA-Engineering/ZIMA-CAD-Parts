#include <QCoreApplication>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QSettings>
#include <cstdio>
#include "../core/partsquery.h"
#include "../zima-cad-parts.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ZIMA-Construction");
    QCoreApplication::setOrganizationDomain("zima-contruction.cz");
    QCoreApplication::setApplicationName("ZIMA-CAD-Parts");
    QCoreApplication::setApplicationVersion(VERSION);
    QCommandLineParser parser;
    parser.setApplicationDescription("Read-only Parts CLI: list DIRECTORY or params FILE. Output is UTF-8 JSON.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"json", "Output JSON (the default)."});
    parser.addOption({"language", "Metadata language, e.g. cs or en.", "language"});
    parser.addOption({"name", "Case-insensitive filename substring for list.", "text"});
    parser.addOption({"default-proe-versions", "Fallback if filters.ini omits ShowVersions: all or latest.", "mode"});
    parser.addPositionalArgument("command", "list or params");
    parser.addPositionalArgument("path", "Directory for list; existing file/directory for params.");
    const auto fail = [](const QString &message, int code) {
        const QByteArray output = QJsonDocument(QJsonObject{{"error", message}}).toJson(QJsonDocument::Compact) + '\n';
        std::fwrite(output.constData(), 1, size_t(output.size()), stderr);
        return code;
    };
    if (!parser.parse(app.arguments()))
        return fail(parser.errorText(), 2);
    if (parser.isSet("help")) { std::fputs(parser.helpText().toUtf8().constData(), stdout); return 0; }
    if (parser.isSet("version")) { std::puts(VERSION); return 0; }
    const auto arguments = parser.positionalArguments();
    if (arguments.size() != 2 || (arguments[0] != "list" && arguments[0] != "params"))
        return fail("Expected list DIRECTORY or params FILE. Use --help.", 2);
    if (arguments[0] == "params" && parser.isSet("name"))
        return fail("--name applies only to list", 2);
    QSettings settings;
    const QString language = parser.isSet("language") ? parser.value("language") : settings.value("LanguageMetadata", "en").toString();
    if (language.isEmpty())
        return fail("Metadata language must not be empty", 2);
    bool showVersions = !settings.value("PartFilters/ProE/Enabled", true).toBool()
        || settings.value("PartFilters/ProE/versions", true).toBool();
    if (parser.isSet("default-proe-versions")) {
        const auto mode = parser.value("default-proe-versions");
        if (mode != "all" && mode != "latest")
            return fail("Expected all or latest for --default-proe-versions", 2);
        showVersions = mode == "all";
    }
    try {
        QJsonObject result;
        if (arguments[0] == "list") {
            result = PartsCore::listParts(arguments[1], language, showVersions, parser.value("name"));
        } else {
            QFileInfo file(arguments[1]);
            if (!file.exists())
                return fail("Part does not exist: " + file.absoluteFilePath(), 3);
            PartsCore::DirectorySnapshot metadata(file.absolutePath());
            result = {{"schemaVersion", 1}, {"language", language}, {"part", metadata.part(file, language)}};
        }
        const QByteArray output = QJsonDocument(result).toJson(QJsonDocument::Indented);
        return std::fwrite(output.constData(), 1, size_t(output.size()), stdout) == size_t(output.size()) ? 0 : 4;
    } catch (const QString &error) {
        return fail(error, 3);
    }
}
