#include "partscommand.h"
#include "partsquery.h"
#include "partstools.h"
#include "partsupdatecommand.h"
#include "../zima-cad-parts.h"
#include <QCommandLineParser>
#include <QDir>

QStringList PartsCore::splitCommand(const QString &line)
{
    QStringList result;
    QString word;
    QChar quote;
    bool started = false;
    for (const auto c : line) {
        if (!quote.isNull()) {
            if (c == quote) quote = QChar();
            else word += c;
        } else if (c == '\'' || c == '"') {
            quote = c;
            started = true;
        } else if (c.isSpace()) {
            if (started) { result.append(word); word.clear(); started = false; }
        } else { word += c; started = true; }
    }
    if (!quote.isNull()) throw QString("Unclosed quote");
    if (started) result.append(word);
    return result;
}

PartsCore::CommandResult PartsCore::executeCommand(const QStringList &arguments, const CommandContext &context,
    ToolPlan *previewPlan, std::function<QString(const QString &)> authorizePath)
{
    const auto fail = [](const QString &error, int code) { return CommandResult{{}, {}, error, code}; };
    if (!arguments.isEmpty() && arguments[0] == "update") {
        if (authorizePath) return fail("Use Settings > Updates to manage application updates", 2);
        return executeUpdateCommand(arguments.mid(1));
    }
    QCommandLineParser parser;
    parser.setApplicationDescription("Parts commands: list, params, ps2pdf, ptc-clean, zima-clean, step-edit, update. Tools preview by default; --apply executes. UTF-8 JSON output.");
    parser.addHelpOption();
    parser.addOption({"apply", "Execute a tool operation (otherwise preview only)."});
    parser.addOption({"recursive", "Include subdirectories, excluding 0000-index and links."});
    parser.addOption({"patterns-only", "PTC Cleaner: use masks only, without old-version cleanup."});
    parser.addOption({"mask", "PTC Cleaner: additional filename wildcard; repeat to add more.", "pattern"});
    parser.addOption({"output-dir", "PDF destination directory; default beside each input.", "directory"});
    parser.addOption({"delete-source", "PDF conversion: delete each source only after its PDF is saved."});
    parser.addOption({"set", "STEP field assignment, e.g. author=Name; repeat for more fields.", "field=value"});
    parser.addVersionOption();
    parser.addOption({"json", "Output JSON (the default)."});
    parser.addOption({"language", "Metadata language, e.g. cs or en.", "language"});
    parser.addOption({"name", "Case-insensitive filename substring for list.", "text"});
    parser.addOption({"default-proe-versions", "Fallback if filters.ini omits ShowVersions: all or latest.", "mode"});
    parser.addPositionalArgument("command", "help, list, params, ps2pdf, ptc-clean, zima-clean, step-edit or update");
    parser.addPositionalArgument("path", "Directory or part path. The panel supplies the active directory.");
    if (!parser.parse(QStringList{"ZIMA-CAD-Parts-cli"} + arguments))
        return fail(parser.errorText(), 2);
    if (parser.isSet("help") || arguments == QStringList{"help"})
        return {{}, parser.helpText(), {}, 0};
    if (parser.isSet("version"))
        return {{}, QStringLiteral(VERSION) + '\n', {}, 0};
    if (previewPlan && parser.isSet("apply")) return fail("Use the preview plan and request user approval to apply", 2);
    auto positional = parser.positionalArguments();
    const bool tool = !positional.isEmpty() && QStringList{"ps2pdf", "ptc-clean", "zima-clean", "step-edit"}.contains(positional[0]);
    if (tool) {
        if (positional.size() == 1 && !context.directory.isEmpty()) positional.append(context.directory);
        if (positional.size() != 2 || positional[1].isEmpty()) return fail("Expected TOOL PATH", 2);
        for (const auto &option : {"language", "name", "default-proe-versions"})
            if (parser.isSet(option)) return fail("Option applies only to list/params: " + QString(option), 2);
        if ((parser.isSet("mask") || parser.isSet("patterns-only")) && positional[0] != "ptc-clean") return fail("Cleaner option used for another tool", 2);
        if (parser.isSet("output-dir") && positional[0] != "ps2pdf") return fail("--output-dir applies to ps2pdf", 2);
        if (parser.isSet("delete-source") && positional[0] != "ps2pdf") return fail("--delete-source applies to ps2pdf", 2);
        if (parser.isSet("set") && positional[0] != "step-edit") return fail("--set applies to step-edit", 2);
        const auto resolve = [&context](const QString &path) {
            return QDir::isAbsolutePath(path) || context.directory.isEmpty() ? path : QDir(context.directory).filePath(path);
        };
        ToolRequest request;
        request.tool = positional[0]; request.path = resolve(positional[1]);
        request.recursive = parser.isSet("recursive");
        request.oldVersions = !parser.isSet("patterns-only");
        request.patterns = parser.values("mask");
        if (parser.isSet("output-dir")) request.outputDirectory = resolve(parser.value("output-dir"));
        request.deleteSourcesAfterConversion = parser.isSet("delete-source");
        for (const auto &assignment : parser.values("set")) {
            const int equals = assignment.indexOf('=');
            if (equals < 1) return fail("Expected --set field=value", 2);
            request.fields[assignment.left(equals)] = assignment.mid(equals + 1);
        }
        try {
            if (authorizePath) {
                request.path = authorizePath(request.path);
                if (!request.outputDirectory.isEmpty()) request.outputDirectory = authorizePath(request.outputDirectory);
            }
            const auto plan = planTool(request);
            if (previewPlan) *previewPlan = plan;
            auto data = parser.isSet("apply") ? applyTool(plan) : describePlan(plan);
            const bool failed = !data["failed"].toArray().isEmpty() || !data["skipped"].toArray().isEmpty();
            return {data, {}, failed && parser.isSet("apply") ? "Some files could not be processed; see result" : QString(),
                failed && parser.isSet("apply") ? 3 : 0};
        } catch (const QString &error) { return fail(error, 3); }
    }
    for (const auto &option : {"apply", "recursive", "patterns-only", "mask", "output-dir", "delete-source", "set"})
        if (parser.isSet(option)) return fail("Option applies only to tools: " + QString(option), 2);
    if (positional.size() == 1 && positional[0] == "list" && !context.directory.isEmpty())
        positional.append(context.directory);
    if (positional.size() != 2 || (positional[0] != "list" && positional[0] != "params"))
        return fail("Expected list DIRECTORY or params FILE. Use help.", 2);
    if (positional[1].isEmpty())
        return fail("Path must not be empty", 2);
    if (positional[0] == "params" && parser.isSet("name"))
        return fail("--name applies only to list", 2);
    const auto language = parser.isSet("language") ? parser.value("language") : context.language;
    if (language.isEmpty()) return fail("Metadata language must not be empty", 2);
    bool versions = context.showVersions;
    if (parser.isSet("default-proe-versions")) {
        const auto mode = parser.value("default-proe-versions");
        if (mode != "all" && mode != "latest") return fail("Expected all or latest for --default-proe-versions", 2);
        versions = mode == "all";
    }
    QString path = QDir::isAbsolutePath(positional[1]) || context.directory.isEmpty()
        ? positional[1] : QDir(context.directory).filePath(positional[1]);
    try {
        if (authorizePath) path = authorizePath(path);
        if (positional[0] == "list")
            return {listParts(path, language, versions, parser.value("name")), {}, {}, 0};
        QFileInfo file(path);
        if (!file.exists()) return fail("Part does not exist: " + file.absoluteFilePath(), 3);
        DirectorySnapshot metadata(file.absolutePath());
        return {{{"schemaVersion", 1}, {"language", language}, {"part", metadata.part(file, language)}}, {}, {}, 0};
    } catch (const QString &error) { return fail(error, 3); }
}
