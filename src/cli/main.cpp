#include <QCoreApplication>
#include <QJsonDocument>
#include <QSettings>
#include <cstdio>
#include "../core/partscommand.h"
#include "../zima-cad-parts.h"
#include "../update/installationclient.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ZIMA-Construction");
    QCoreApplication::setOrganizationDomain("zima-contruction.cz");
    QCoreApplication::setApplicationName("ZIMA-CAD-Parts");
    QCoreApplication::setApplicationVersion(VERSION);
    QString registrationError;
    if (!registerPartsInstance(&registrationError)) {
        const auto bytes = QJsonDocument(QJsonObject{{"error", registrationError}}).toJson(QJsonDocument::Compact);
        std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stderr); return 3;
    }
    QSettings settings;
    PartsCore::CommandContext context;
    context.language = settings.value("LanguageMetadata", "en").toString();
    context.showVersions = !settings.value("PartFilters/ProE/Enabled", true).toBool()
        || settings.value("PartFilters/ProE/versions", true).toBool();
    const auto result = PartsCore::executeCommand(app.arguments().mid(1), context);
    auto errorData = result.data;
    errorData["error"] = result.error;
    const QByteArray output = result.code ? QJsonDocument(errorData).toJson(QJsonDocument::Compact) + '\n'
        : !result.text.isEmpty() ? result.text.toUtf8() : QJsonDocument(result.data).toJson(QJsonDocument::Indented);
    if (std::fwrite(output.constData(), 1, size_t(output.size()), result.code ? stderr : stdout) != size_t(output.size()))
        return 4;
    return result.code;
}
