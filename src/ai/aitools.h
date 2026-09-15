// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef AITOOLS_H
#define AITOOLS_H
#include "core/partscommand.h"
#include "core/partstools.h"

namespace PartsAi {
QJsonArray toolDefinitions();
QString instructions();
QString hostInstructions();
QJsonArray referenceData(const QStringList &paths);
QString quotedPath(const QString &path);
QJsonArray promptReferences(const QString &text);
QString checkedPath(const QString &path, const PartsCore::CommandContext &context, const QJsonArray &references = {});
QJsonObject contextData(const PartsCore::CommandContext &context, const QJsonArray &references = {});
struct ToolResult {
    QJsonObject data;
    PartsCore::ToolPlan plan;
    bool success = true;
};
ToolResult runTool(const QString &name, const QJsonObject &arguments, const PartsCore::CommandContext &context,
    const QJsonArray &references = {});
}
#endif
