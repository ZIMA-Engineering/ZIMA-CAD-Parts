#include "partsupdatecommand.h"
#include "../update/installationclient.h"
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QProcess>
#include <QThread>
#include <QUuid>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

PartsCore::CommandResult PartsCore::executeUpdateCommand(const QStringList &arguments)
{
    const auto fail = [](const QString &error, int code) { return CommandResult{{}, {}, error, code}; };
    QCommandLineParser parser;
    parser.setApplicationDescription("Updates: check, status, download, install, rollback. Download prepares only. Install/rollback require --apply and restart Parts asynchronously.");
    parser.addHelpOption();
    parser.addOption({"target", "Release version for download or install", "YYYYMMDDNN"});
    parser.addOption({"apply", "Confirm installation or rollback and restart Parts"});
    parser.addOption({"json", "Output JSON (default)"});
    parser.addPositionalArgument("command", "check, status, download, install, rollback");
    if (!parser.parse(QStringList{"update"} + arguments)) return fail(parser.errorText(), 2);
    if (parser.isSet("help") || arguments == QStringList{"help"}) return {{}, parser.helpText(), {}, 0};
    const auto positional = parser.positionalArguments();
    if (positional.size() != 1 || !QStringList{"check", "status", "download", "install", "rollback"}.contains(positional[0]))
        return fail("Expected update check, status, download, install or rollback; use update --help", 2);
    const auto command = positional[0];
    if (parser.isSet("target") && command != "download" && command != "install") return fail("--target applies to download/install", 2);
    if (command == "install" && !parser.isSet("target")) return fail("Install requires --target YYYYMMDDNN", 2);
    if (parser.isSet("apply") && command != "install" && command != "rollback") return fail("--apply applies to install/rollback", 2);
    QStringList args{command};
    const auto root = partsInstallationRoot();
    if (!root.isEmpty()) args << "--root" << root;
    if (parser.isSet("target")) args << "--target" << parser.value("target");
    QProcess process;
    process.setProgram(partsUpdateExecutable());
#ifdef Q_OS_WIN
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) { args->flags |= CREATE_NO_WINDOW; });
#endif
    if (parser.isSet("apply")) {
        if (root.isEmpty()) return fail("Use the managed distribution to install updates", 3);
        const auto operation = QUuid::createUuid().toString(QUuid::WithoutBraces);
        args << "--apply" << "--operation" << operation;
        process.setArguments(args);
        if (!process.startDetached()) return fail("Cannot start updater", 3);
        return {{{"schemaVersion", 1}, {"status", "activation-requested"}, {"operation", operation},
                 {"installationRoot", root}, {"requestApplicationClose", true}}, {}, {}, 0};
    }
    process.setArguments(args); process.start();
    if (!process.waitForStarted(5000)) return fail("Cannot start updater", 3);
    QByteArray buffer;
    QJsonObject result;
    while (process.state() != QProcess::NotRunning) {
        process.waitForFinished(250);
        buffer += process.readAllStandardOutput();
        process.readAllStandardError();
        if (QThread::currentThread()->isInterruptionRequested() || buffer.size() > 4 * 1024 * 1024) {
            process.kill(); process.waitForFinished(1000); return fail("Update operation cancelled", 3);
        }
        while (buffer.contains('\n')) {
            const auto index = buffer.indexOf('\n');
            const auto object = QJsonDocument::fromJson(buffer.left(index)).object();
            buffer.remove(0, index + 1);
            if (object["event"] == "result") result = object;
        }
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() || result.isEmpty())
        return fail(result["error"].toString("Update operation failed"), 3);
    return {result, {}, {}, 0};
}
