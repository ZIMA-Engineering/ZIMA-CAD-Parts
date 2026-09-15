#ifndef PARTSCOMMAND_H
#define PARTSCOMMAND_H
#include <QJsonObject>
#include <QStringList>
#include <functional>
namespace PartsCore {
struct ToolPlan;
struct CommandContext {
    QString directory;
    QString language = "en";
    bool showVersions = true;
    QString workingDirectory;
};
struct CommandResult { QJsonObject data; QString text; QString error; int code = 0; };
QStringList splitCommand(const QString &line);
CommandResult executeCommand(const QStringList &arguments, const CommandContext &context,
    ToolPlan *previewPlan = nullptr, std::function<QString(const QString &)> authorizePath = {});
}
#endif
