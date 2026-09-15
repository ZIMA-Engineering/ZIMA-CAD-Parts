#include "updatecore.h"
#include "../zima-cad-parts.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <cstdio>

static void print(const QJsonObject &object)
{
    const auto bytes = PartsUpdate::canonical(object);
    std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout); std::fflush(stdout);
}
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("ZIMA-Construction"); app.setOrganizationDomain("zima-contruction.cz");
    app.setApplicationName("ZIMA-CAD-Parts"); app.setApplicationVersion(VERSION);
#ifdef PARTS_UPDATE_TESTING
    const auto testState = qEnvironmentVariable("ZCP_UPDATE_TEST_STATE");
    if (!testState.isEmpty()) {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, testState);
    }
#endif
    QCommandLineParser parser; parser.addHelpOption();
    parser.addOption({"root", "Managed installation root", "directory"});
    parser.addOption({"target", "Target version", "YYYYMMDDNN"});
    parser.addOption({"operation", "Installation operation ID", "id"});
    parser.addOption({"apply", "Activate a verified update or rollback"});
    parser.addOption({"version", "Print binary version"});
#ifdef PARTS_UPDATE_TESTING
    parser.addOption({"archive", "Fixture archive", "file"});
    parser.addOption({"manifest", "Fixture manifest", "file"});
    parser.addOption({"signature", "Fixture signature", "file"});
#endif
    parser.addPositionalArgument("command", "check, download, install, rollback, status, launch");
    if (!parser.parse(app.arguments())) { print({{"error", parser.errorText()}}); return 2; }
    if (parser.isSet("version")) { std::printf("%s\n", VERSION); return 0; }
    if (parser.isSet("help")) { std::printf("%s", parser.helpText().toUtf8().constData()); return 0; }
    const auto args = parser.positionalArguments();
    if (args.size() != 1) { print({{"error", "Expected one update command"}}); return 2; }
    const auto root = parser.isSet("root") ? QDir(parser.value("root")).absolutePath() : PartsUpdate::installationRoot();
    const auto saveResult = [&](QJsonObject result) {
        if (root.isEmpty() || !QStringList{"install", "rollback", "launch"}.contains(args[0])) return;
        if (!parser.isSet("apply") && args[0] != "launch") return;
        try {
            PartsUpdate::assertManaged(root);
            QDir().mkpath(PartsUpdate::child(root, ".updates"));
            result["operation"] = parser.value("operation");
            PartsUpdate::writeJson(PartsUpdate::child(root, ".updates/" + PartsUpdate::platform() + "-result.json"), result);
        } catch (...) {} // Reporting must not replace the original result.
    };
    try {
        const auto command = args[0];
        QJsonObject result;
        const auto progress = [](const QJsonObject &data) { auto object = data; object["event"] = "progress"; print(object); };
        if (command == "check") result = PartsUpdate::check(root);
        else if (command == "status") result = root.isEmpty() ? QJsonObject{{"status", "unmanaged"}} : PartsUpdate::status(root);
        else if (command == "launch") return PartsUpdate::launch(root);
        else if (command == "download") {
            const auto offer = PartsUpdate::check(root);
            if (offer["status"] != "available" || (parser.isSet("target") && offer["availableVersion"] != parser.value("target"))) throw QString("Requested update is no longer available");
            result = PartsUpdate::download(root, offer, progress);
        } else if (command == "install") {
            if (!parser.isSet("apply")) result = {{"status", "confirmation-required"}, {"version", parser.value("target")}};
            else result = PartsUpdate::activate(root, parser.value("target"), progress);
        } else if (command == "rollback") {
            if (!parser.isSet("apply")) result = {{"status", "confirmation-required"}};
            else result = PartsUpdate::rollback(root, progress);
#ifdef PARTS_UPDATE_TESTING
        } else if (command == "prepare") {
            QFile payload(parser.value("manifest")), sig(parser.value("signature"));
            if (!payload.open(QIODevice::ReadOnly) || !sig.open(QIODevice::ReadOnly)) throw QString("Missing fixture manifest");
            result = PartsUpdate::prepare(root, parser.value("archive"), payload.readAll(), sig.readAll(), progress);
#endif
        } else throw QString("Unknown update command");
        saveResult(result);
        result["schemaVersion"] = 1; result["event"] = "result"; print(result); return 0;
    } catch (const QString &error) { saveResult({{"status", "error"}, {"error", error}}); print({{"schemaVersion", 1}, {"event", "result"}, {"status", "error"}, {"error", error}}); return 3; }
}
