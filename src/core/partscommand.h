#ifndef PARTSCOMMAND_H
#define PARTSCOMMAND_H
#include <QJsonObject>
#include <QStringList>
namespace PartsCore {
struct CommandContext { QString directory; QString language = "en"; bool showVersions = true; };
struct CommandResult { QJsonObject data; QString text; QString error; int code = 0; };
QStringList splitCommand(const QString &line);
CommandResult executeCommand(const QStringList &arguments, const CommandContext &context);
}
#endif
