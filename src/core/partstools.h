#ifndef PARTSTOOLS_H
#define PARTSTOOLS_H
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QList>
#include <functional>

namespace PartsCore {
struct ToolRequest {
    QString tool;
    QString path;
    bool recursive = false;
    bool oldVersions = true;
    bool deleteSourcesAfterConversion = false;
    QStringList patterns;
    QString outputDirectory;
    QJsonObject fields;
};
struct ToolItem {
    QString path, hash, output, keeper, keeperHash;
    QByteArray replacement;
    QJsonObject fields;
};
struct ToolPlan {
    ToolRequest request;
    QList<ToolItem> items;
    QJsonArray skipped;
};
QString ghostscriptExecutable();
ToolPlan planTool(const ToolRequest &request);
QJsonObject describePlan(const ToolPlan &plan);
// Called on the worker thread after each successfully completed file.
using ToolCompleted = std::function<void(const QString &path)>;
QJsonObject applyTool(const ToolPlan &plan, const ToolCompleted &completed = {});
}
#endif
