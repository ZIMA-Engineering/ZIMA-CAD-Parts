// Built-in successors to ZIMA-PS2PDF, ZIMA-PTC-Cleaner and ZIMA-STEP-Edit.
// Original utilities: Peter Holak (2008), Jakub Skokan (2011-2012).
// New shared implementation: ZIMA-Engineering, GPL-3.0-or-later.
#include "partstools.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QThread>
#include <QProcess>
#include <QProcessEnvironment>
#include <QUuid>
#include <QMap>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <shobjidl.h>
#include <wrl/client.h>
#include <wrl/implements.h>
#endif

namespace {
void checkCanceled()
{
    if (QThread::currentThread()->isInterruptionRequested()) throw QString("Cancelled");
}
void checkPath(const QString &path)
{
    QFileInfo entry(path);
    for (;;) {
        if (entry.isSymLink() || entry.isJunction()) throw QString("Links are not supported: ") + path;
        if (entry.fileName().compare("0000-index", Qt::CaseInsensitive) == 0)
            throw QString("System directory is excluded: ") + path;
        const QString parent = entry.absolutePath();
        if (parent == entry.absoluteFilePath()) break;
        entry.setFile(parent);
    }
}
QString fingerprint(const QString &path)
{
    checkPath(path);
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) throw f.errorString() + ": " + path;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!f.atEnd()) {
        checkCanceled();
        auto bytes = f.read(1024 * 1024);
        if (f.error() != QFileDevice::NoError) throw f.errorString();
        hash.addData(bytes);
    }
    return QString::fromLatin1(hash.result().toHex());
}
bool locked(const QString &path)
{
    const QString file = QDir(QFileInfo(path).absolutePath()).filePath("0000-index/metadata.ini");
    QSettings s(file, QSettings::IniFormat);
    s.setFallbacksEnabled(false);
    const bool result = s.value("Directory/PreventRemoval", false).toBool();
    if (s.status() != QSettings::NoError) throw QString("Cannot read directory lock: ") + file;
    return result;
}
QStringList collectFiles(const PartsCore::ToolRequest &request)
{
    if (request.path.trimmed().isEmpty()) throw QString("Source path must not be empty");
    const QFileInfo root(request.path);
    checkPath(root.absoluteFilePath());
    if (!root.exists() || !root.isReadable()) throw QString("Path is not readable: ") + request.path;
    if (root.isFile()) return {root.absoluteFilePath()};
    QStringList result, pending{root.absoluteFilePath()};
    while (!pending.isEmpty()) {
        checkCanceled();
        const auto directory = pending.takeLast();
        checkPath(directory);
        if (!QFileInfo(directory).isReadable()) throw QString("Directory is not readable: ") + directory;
        const auto entries = QDir(directory).entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden
            | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
        for (const auto &entry : entries) {
            if (entry.isJunction() || entry.fileName().compare("0000-index", Qt::CaseInsensitive) == 0) continue;
            if (entry.isFile()) result.append(entry.absoluteFilePath());
            else if (entry.isDir() && request.recursive) pending.append(entry.absoluteFilePath());
        }
    }
    return result;
}
#ifdef Q_OS_WIN
QString trashError(HRESULT result)
{
    return QString("Cannot move file to trash (Windows error 0x%1)")
        .arg(quint32(result), 8, 16, QLatin1Char('0'));
}
class TrashProgress final : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IFileOperationProgressSink>
{
public:
    TrashProgress(const PartsCore::ToolItem &value, const PartsCore::ToolCompleted &callback)
        : item(value), onCompleted(callback) {}
    PartsCore::ToolItem item;
    PartsCore::ToolCompleted onCompleted;
    QString error, trash;
    bool completed = false;

    IFACEMETHODIMP PreDeleteItem(DWORD flags, IShellItem *) override
    {
        try {
            checkCanceled();
            // Refuse the Shell's permanent-delete fallback, including on shares.
            if (!(flags & TSF_DELETE_RECYCLE_IF_POSSIBLE)) throw QString("System trash is unavailable");
            if (fingerprint(item.path) != item.hash) throw QString("File changed since preview");
            if (locked(item.path)) throw QString("Directory is locked");
            if (!item.keeper.isEmpty() && fingerprint(item.keeper) != item.keeperHash)
                throw QString("Retained version changed since preview");
            return S_OK;
        } catch (const QString &message) {
            error = message;
            return E_ABORT;
        }
    }
    IFACEMETHODIMP PostDeleteItem(DWORD, IShellItem *, HRESULT result, IShellItem *recycled) override
    {
        completed = SUCCEEDED(result) && recycled && !QFileInfo::exists(item.path);
        if (completed) {
            PWSTR value = nullptr;
            if (SUCCEEDED(recycled->GetDisplayName(SIGDN_FILESYSPATH, &value))) {
                trash = QDir::fromNativeSeparators(QString::fromWCharArray(value));
                CoTaskMemFree(value);
            }
            if (onCompleted) onCompleted(item.path);
        } else if (error.isEmpty()) {
            error = trashError(result);
        }
        return S_OK;
    }
    IFACEMETHODIMP StartOperations() override { return S_OK; }
    IFACEMETHODIMP FinishOperations(HRESULT) override { return S_OK; }
    IFACEMETHODIMP PreRenameItem(DWORD, IShellItem *, LPCWSTR) override { return S_OK; }
    IFACEMETHODIMP PostRenameItem(DWORD, IShellItem *, LPCWSTR, HRESULT, IShellItem *) override { return S_OK; }
    IFACEMETHODIMP PreMoveItem(DWORD, IShellItem *, IShellItem *, LPCWSTR) override { return S_OK; }
    IFACEMETHODIMP PostMoveItem(DWORD, IShellItem *, IShellItem *, LPCWSTR, HRESULT, IShellItem *) override { return S_OK; }
    IFACEMETHODIMP PreCopyItem(DWORD, IShellItem *, IShellItem *, LPCWSTR) override { return S_OK; }
    IFACEMETHODIMP PostCopyItem(DWORD, IShellItem *, IShellItem *, LPCWSTR, HRESULT, IShellItem *) override { return S_OK; }
    IFACEMETHODIMP PreNewItem(DWORD, IShellItem *, LPCWSTR) override { return S_OK; }
    IFACEMETHODIMP PostNewItem(DWORD, IShellItem *, LPCWSTR, LPCWSTR, DWORD, HRESULT, IShellItem *) override { return S_OK; }
    IFACEMETHODIMP UpdateProgress(UINT, UINT) override
    {
        return QThread::currentThread()->isInterruptionRequested() ? E_ABORT : S_OK;
    }
    IFACEMETHODIMP ResetTimer() override { return S_OK; }
    IFACEMETHODIMP PauseTimer() override { return S_OK; }
    IFACEMETHODIMP ResumeTimer() override { return S_OK; }
};
QJsonObject recycleWindows(const PartsCore::ToolPlan &plan, const PartsCore::ToolCompleted &onCompleted)
{
    struct Apartment {
        HRESULT status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        ~Apartment() { if (SUCCEEDED(status)) CoUninitialize(); }
    } apartment;
    if (FAILED(apartment.status)) throw trashError(apartment.status);
    Microsoft::WRL::ComPtr<IFileOperation> operation;
    auto result = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(operation.GetAddressOf()));
    if (FAILED(result)) throw trashError(result);
    result = operation->SetOperationFlags(FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI
        | FOF_NO_CONNECTED_ELEMENTS | FOFX_RECYCLEONDELETE | FOFX_ADDUNDORECORD);
    if (FAILED(result)) throw trashError(result);
    QList<Microsoft::WRL::ComPtr<TrashProgress>> sinks;
    for (const auto &item : plan.items) {
        auto sink = Microsoft::WRL::Make<TrashProgress>(item, onCompleted);
        sinks.append(sink);
        try {
            checkCanceled();
            checkPath(item.path);
            if (!QFileInfo(item.path).isFile()) throw QString("Source file is missing");
            Microsoft::WRL::ComPtr<IShellItem> source;
            const auto path = QDir::toNativeSeparators(item.path);
            result = SHCreateItemFromParsingName(reinterpret_cast<LPCWSTR>(path.utf16()), nullptr,
                IID_PPV_ARGS(source.GetAddressOf()));
            if (FAILED(result)) throw trashError(result);
            result = operation->DeleteItem(source.Get(), sink.Get());
            if (FAILED(result)) throw trashError(result);
        } catch (const QString &message) { sink->error = message; }
    }
    // One Shell transaction for the whole selection, on the tool's worker thread.
    // PreDeleteItem validates each file immediately before the Shell recycles it.
    const auto performed = operation->PerformOperations();
    BOOL aborted = FALSE;
    const auto abortStatus = operation->GetAnyOperationsAborted(&aborted);
    QJsonArray completed, failed;
    for (const auto &sink : sinks) {
        if (sink->completed) {
            completed.append(QJsonObject{{"path", sink->item.path}, {"trash", sink->trash}});
        } else {
            if (sink->error.isEmpty()) {
                sink->error = QThread::currentThread()->isInterruptionRequested() ? "Cancelled"
                    : FAILED(performed) ? trashError(performed)
                    : FAILED(abortStatus) ? trashError(abortStatus)
                    : "Not processed because recycling was interrupted";
            }
            failed.append(QJsonObject{{"path", sink->item.path}, {"reason", sink->error}});
        }
    }
    return {{"schemaVersion", 1}, {"tool", plan.request.tool}, {"preview", false},
        {"completed", completed}, {"failed", failed}, {"skipped", plan.skipped},
        {"cancelled", QThread::currentThread()->isInterruptionRequested() || bool(aborted)}};
}
#endif
const QStringList fieldNames{"name", "date", "author", "organization", "preprocessor", "system", "authorization"};
struct StepHeader { int start = -1, end = -1; QStringList raw; QJsonObject fields; };
// Lexical scan handles comments, apostrophes, commas and nested author lists.
QStringList splitStep(const QString &text)
{
    QStringList values;
    int start = 0, depth = 0;
    bool quoted = false, comment = false;
    for (int i = 0; i < text.size(); ++i) {
        const auto c = text[i];
        if (comment) { if (c == '*' && text.mid(i, 2) == "*/") { comment = false; ++i; } continue; }
        if (quoted) {
            if (c == '\'') { if (text.mid(i, 2) == "''") ++i; else quoted = false; }
            continue;
        }
        if (text.mid(i, 2) == "/*") { comment = true; ++i; }
        else if (c == '\'') quoted = true;
        else if (c == '(') ++depth;
        else if (c == ')') --depth;
        else if (c == ',' && depth == 0) { values.append(text.mid(start, i - start).trimmed()); start = i + 1; }
        if (depth < 0) throw QString("Malformed STEP header");
    }
    if (quoted || comment || depth != 0) throw QString("Malformed STEP header");
    values.append(text.mid(start).trimmed());
    return values;
}
QString stepString(const QString &text)
{
    QString result = "'";
    for (const QChar c : text) {
        if (c == '\'') result += "''";
        else if (c == '\\') result += "\\\\";
        else if (c.unicode() >= 32 && c.unicode() < 127) result += c;
        else result += QString("\\X2\\%1\\X0\\").arg(uint(c.unicode()), 4, 16, QLatin1Char('0')).toUpper();
    }
    return result + "'";
}
QString decodeString(QString value)
{
    value = value.trimmed();
    if (!value.startsWith('\'') || !value.endsWith('\'')) throw QString("Malformed STEP string");
    value = value.mid(1, value.size() - 2);
    QString out;
    for (int i = 0; i < value.size(); ++i) {
        if (value.mid(i, 2) == "''") { out += '\''; ++i; }
        else if (value.mid(i, 2) == "\\\\") { out += '\\'; ++i; }
        else if (value.mid(i, 4) == "\\X2\\" || value.mid(i, 4) == "\\X4\\") {
            const int width = value[i + 2] == '2' ? 4 : 8;
            const int end = value.indexOf("\\X0\\", i + 4);
            if (end < 0 || (end - i - 4) % width) throw QString("Invalid STEP Unicode escape");
            for (int j = i + 4; j < end; j += width) {
                bool ok;
                uint cp = value.mid(j, width).toUInt(&ok, 16);
                if (!ok || cp > 0x10ffff) throw QString("Invalid STEP Unicode escape");
                if (width == 4) out += QChar(ushort(cp));
                else { const char32_t u = cp; out += QString::fromUcs4(&u, 1); }
            }
            i = end + 3;
        } else out += value[i];
    }
    return out;
}
StepHeader readStep(const QByteArray &bytes)
{
    const QString text = QString::fromLatin1(bytes);
    if (!text.trimmed().startsWith("ISO-10303-21;")) throw QString("Not an ISO STEP file");
    StepHeader h;
    bool quoted = false, comment = false, inHeader = false;
    int record = 0;
    for (int i = 0; i < text.size(); ++i) {
        if ((i % 65536) == 0) checkCanceled();
        if (comment) { if (text.mid(i, 2) == "*/") { comment = false; ++i; } continue; }
        const auto c = text[i];
        if (quoted) { if (c == '\'') { if (text.mid(i, 2) == "''") ++i; else quoted = false; } continue; }
        if (text.mid(i, 2) == "/*") { comment = true; ++i; continue; }
        if (c == '\'') { quoted = true; continue; }
        if (c != ';') continue;
        QString stmt = text.mid(record, i - record).trimmed();
        stmt.remove(QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption));
        stmt = stmt.trimmed();
        if (stmt == "HEADER") inHeader = true;
        else if (inHeader && stmt == "ENDSEC") break;
        else if (inHeader && stmt.startsWith("FILE_NAME")) {
            // Locate the token outside comments to preserve surrounding bytes.
            int start = record;
            while (start < i) {
                if (text[start].isSpace()) { ++start; continue; }
                if (text.mid(start, 2) == "/*") { start = text.indexOf("*/", start + 2) + 2; continue; }
                break;
            }
            QString body = text.mid(start + 9, i - start - 9).trimmed();
            if (!body.startsWith('(') || !body.endsWith(')')) throw QString("Malformed FILE_NAME");
            h.start = start; h.end = i + 1;
            h.raw = splitStep(body.mid(1, body.size() - 2));
            if (h.raw.size() != 7) throw QString("FILE_NAME must have seven fields");
            for (int n = 0; n < 7; ++n) {
                if (n == 2 || n == 3) {
                    auto list = h.raw[n].trimmed();
                    if (!list.startsWith('(') || !list.endsWith(')')) throw QString("Malformed STEP author list");
                    QJsonArray array;
                    if (!list.mid(1, list.size() - 2).trimmed().isEmpty())
                        for (const auto &item : splitStep(list.mid(1, list.size() - 2))) array.append(decodeString(item));
                    h.fields[fieldNames[n]] = array;
                } else h.fields[fieldNames[n]] = decodeString(h.raw[n]);
            }
            return h;
        }
        record = i + 1;
    }
    throw QString("STEP FILE_NAME header not found");
}
void convertPdf(const PartsCore::ToolItem &item)
{
    const auto exe = PartsCore::ghostscriptExecutable();
    if (exe.isEmpty()) throw QString("Ghostscript runtime is missing");
    QTemporaryDir tmp(QDir(QFileInfo(item.output).absolutePath()).filePath(".parts-pdf-XXXXXX"));
    if (!tmp.isValid()) throw QString("Cannot create PDF temporary directory");
    const QString output = tmp.filePath("output.pdf");
    QProcess process;
    auto env = QProcessEnvironment::systemEnvironment();
    for (const auto &key : {"GS_OPTIONS", "GS_LIB", "GS_FONTPATH"}) env.remove(key);
    process.setProcessEnvironment(env);
    process.setWorkingDirectory(tmp.path());
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(exe, {"-dSAFER", "-dBATCH", "-dNOPAUSE", "-sDEVICE=pdfwrite", "-dCompatibilityLevel=1.7",
        "-sOutputFile=" + QDir::toNativeSeparators(output), "-f", QDir::toNativeSeparators(item.path)});
    if (!process.waitForStarted()) throw QString("Cannot start Ghostscript: ") + process.errorString();
    QByteArray log;
    while (!process.waitForFinished(100)) {
        log = (log + process.readAll()).right(16000);
        if (QThread::currentThread()->isInterruptionRequested()) { process.kill(); process.waitForFinished(); throw QString("Cancelled"); }
    }
    log = (log + process.readAll()).right(16000);
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode()) throw QString::fromLocal8Bit(log);
    QFile pdf(output);
    if (!pdf.open(QIODevice::ReadOnly) || pdf.read(5) != "%PDF-") throw QString("Ghostscript produced no valid PDF");
    if (!pdf.seek(0)) throw QString("Cannot read generated PDF");
    QSaveFile destination(item.output);
    if (!destination.open(QIODevice::WriteOnly)) throw QString("Cannot save PDF: ") + destination.errorString();
    while (!pdf.atEnd()) {
        checkCanceled();
        const auto bytes = pdf.read(1024 * 1024);
        if (pdf.error() != QFileDevice::NoError) throw QString("Cannot read generated PDF: ") + pdf.errorString();
        if (destination.write(bytes) != bytes.size()) throw QString("Cannot save PDF: ") + destination.errorString();
    }
    if (!destination.commit()) throw QString("Cannot replace PDF: ") + destination.errorString();
}

QString pdfOutputDirectory(const PartsCore::ToolRequest &request)
{
    const QFileInfo source(request.path);
    const QString base = source.isDir() ? source.absoluteFilePath() : source.absolutePath();
    return QDir::isAbsolutePath(request.outputDirectory)
        ? QDir::cleanPath(request.outputDirectory)
        : QDir(base).absoluteFilePath(request.outputDirectory);
}
}

QString PartsCore::ghostscriptExecutable()
{
#ifdef Q_OS_WIN
    const QString relative = "tools/ghostscript/bin/gswin64c.exe";
#else
    const QString relative = "tools/ghostscript/bin/gs";
#endif
    const auto local = QDir(QCoreApplication::applicationDirPath()).filePath(relative);
    if (QFileInfo(local).isExecutable()) return local;
#ifndef Q_OS_WIN
    return QStandardPaths::findExecutable("gs");
#else
    return {};
#endif
}

PartsCore::ToolPlan PartsCore::planTool(const ToolRequest &request)
{
    if (!QStringList{"ps2pdf", "ptc-clean", "zima-clean", "step-edit"}.contains(request.tool)) throw QString("Unknown tool");
    for (auto it = request.fields.begin(); it != request.fields.end(); ++it)
        if (!fieldNames.contains(it.key()) || !it.value().isString()) throw QString("Unknown or invalid STEP field: ") + it.key();
    ToolPlan plan; plan.request = request;
    const auto files = collectFiles(request);
    QMap<QString, QPair<qulonglong, QString>> latest;
    const QRegularExpression version("^(.+)\\.([0-9]+)$");
    const QRegularExpression zimaArchive("^.+\\.(?:prtz|asmz|drwz|frmz|tblz)\\.[0-9]+$",
        QRegularExpression::CaseInsensitiveOption);
    for (const auto &file : files) {
        const auto m = version.match(file);
        bool ok;
        const auto number = m.captured(2).toULongLong(&ok);
        if (m.hasMatch() && ok && (!latest.contains(m.captured(1)) || latest[m.captured(1)].first < number))
            latest[m.captured(1)] = {number, file};
    }
    QMap<QString, int> outputs;
    QMap<QString, QString> previewHashes;
    const auto previewHash = [&previewHashes](const QString &path) {
        const auto found = previewHashes.constFind(path);
        if (found != previewHashes.cend()) return found.value();
        const auto value = fingerprint(path);
        previewHashes.insert(path, value);
        return value;
    };
    for (const auto &path : files) {
        checkCanceled();
        const QFileInfo file(path);
        const auto ext = file.suffix().toLower();
        ToolItem item; item.path = path;
        bool wanted = false;
        if (request.tool == "ptc-clean") {
            const auto m = version.match(path);
            bool ok; const auto number = m.captured(2).toULongLong(&ok);
            if (request.oldVersions && ok && latest.contains(m.captured(1)) && number < latest[m.captured(1)].first) {
                wanted = true; item.keeper = latest[m.captured(1)].second;
            }
            for (const auto &pattern : request.patterns) {
                const auto re = QRegularExpression::fromWildcard(pattern, Qt::CaseSensitive, QRegularExpression::NonPathWildcardConversion);
                if (re.match(file.fileName()).hasMatch()) wanted = true;
            }
        } else if (request.tool == "zima-clean") {
            // Current ZIMA-CAD documents have no number. Every numeric suffix
            // is an archive, even without a current file or above uint64 range.
            wanted = zimaArchive.match(file.fileName()).hasMatch();
        } else if (request.tool == "ps2pdf") wanted = QStringList{"ps", "eps", "plt"}.contains(ext);
        else wanted = ext == "stp" || ext == "step";
        if (!wanted) continue;
        try {
            if ((request.tool == "ptc-clean" || request.tool == "zima-clean"
                || (request.tool == "ps2pdf" && request.deleteSourcesAfterConversion)) && locked(path))
                throw QString("Directory is locked");
            item.hash = previewHash(path);
            if (!item.keeper.isEmpty()) item.keeperHash = previewHash(item.keeper);
            if (request.tool == "ps2pdf") {
                QFile input(path);
                if (!input.open(QIODevice::ReadOnly)) throw input.errorString();
                if (!input.read(1024).contains("%!PS")) throw QString("Not PostScript (PLT may contain HPGL or PCL)");
                const auto directory = request.outputDirectory.isEmpty() ? file.absolutePath() : pdfOutputDirectory(request);
                checkPath(directory);
                if (QFileInfo::exists(directory) && !QFileInfo(directory).isDir()) throw QString("Output path is not a directory");
                item.output = QDir(directory).filePath(file.completeBaseName() + ".pdf");
                checkPath(item.output);
                if (++outputs[item.output.toCaseFolded()] > 1) throw QString("Multiple inputs target the same PDF");
            } else if (request.tool == "step-edit") {
                QFile input(path);
                if (!input.open(QIODevice::ReadOnly)) throw input.errorString();
                const auto bytes = input.readAll();
                if (input.error() != QFileDevice::NoError) throw input.errorString();
                auto h = readStep(bytes); item.fields = h.fields;
                for (auto it = request.fields.begin(); it != request.fields.end(); ++it) {
                    const int index = fieldNames.indexOf(it.key());
                    const QString val = stepString(it.value().toString());
                    h.raw[index] = (index == 2 || index == 3) ? '(' + val + ')' : val;
                }
                if (!request.fields.isEmpty()) item.replacement = bytes.left(h.start)
                    + ("FILE_NAME(" + h.raw.join(",") + ");").toLatin1() + bytes.mid(h.end);
            }
            plan.items.append(item);
        } catch (const QString &error) { checkCanceled(); plan.skipped.append(QJsonObject{{"path", path}, {"reason", error}}); }
    }
    // A collision invalidates every input with this destination, not just the last.
    for (int i = plan.items.size() - 1; i >= 0; --i) {
        if (!plan.items[i].output.isEmpty() && outputs[plan.items[i].output.toCaseFolded()] > 1) {
            plan.skipped.append(QJsonObject{{"path", plan.items[i].path}, {"reason", "Conflicting PDF destination"}});
            plan.items.removeAt(i);
        }
    }
    return plan;
}

QJsonObject PartsCore::describePlan(const ToolPlan &plan)
{
    QJsonArray items;
    for (const auto &item : plan.items) {
        QJsonObject value{{"path", item.path}, {"sha256", item.hash}};
        if (!item.output.isEmpty()) value["output"] = item.output;
        if (!item.keeper.isEmpty()) value["keep"] = item.keeper;
        if (!item.fields.isEmpty()) value["fields"] = item.fields;
        items.append(value);
    }
    return {{"schemaVersion", 1}, {"tool", plan.request.tool}, {"preview", true}, {"items", items},
        {"changes", plan.request.fields}, {"skipped", plan.skipped}};
}

QJsonObject PartsCore::applyTool(const ToolPlan &plan, const ToolCompleted &onCompleted)
{
#ifdef Q_OS_WIN
    if ((plan.request.tool == "ptc-clean" || plan.request.tool == "zima-clean") && !plan.items.isEmpty())
        return recycleWindows(plan, onCompleted);
#endif
    QJsonArray completed, failed;
    if (plan.request.tool == "step-edit" && plan.request.fields.isEmpty()) throw QString("No STEP fields specified");
    for (const auto &item : plan.items) {
        try {
            checkCanceled();
            if (fingerprint(item.path) != item.hash) throw QString("File changed since preview");
            QJsonObject result{{"path", item.path}};
            if (plan.request.tool == "ptc-clean" || plan.request.tool == "zima-clean") {
                if (locked(item.path)) throw QString("Directory is locked");
                if (!item.keeper.isEmpty() && fingerprint(item.keeper) != item.keeperHash) throw QString("Retained version changed since preview");
                QString trash;
                if (!QFile::moveToTrash(item.path, &trash)) throw QString("Cannot move file to trash");
                result["trash"] = trash;
            } else if (plan.request.tool == "ps2pdf") {
                if (plan.request.deleteSourcesAfterConversion && locked(item.path)) throw QString("Directory is locked");
                checkPath(item.output);
                const auto outputDirectory = QFileInfo(item.output).absolutePath();
                if (!QDir().mkpath(outputDirectory)) throw QString("Cannot create PDF output directory: ") + outputDirectory;
                convertPdf(item);
                result["output"] = item.output;
                if (plan.request.deleteSourcesAfterConversion) {
                    if (locked(item.path)) throw QString("Directory is locked");
                    if (fingerprint(item.path) != item.hash) throw QString("File changed during conversion");
                    if (!QFile::remove(item.path)) throw QString("PDF was created, but the source file could not be deleted");
                    result["sourceDeleted"] = true;
                }
            } else if (plan.request.tool == "step-edit") {
                const auto folder = QDir(QFileInfo(item.path).absolutePath()).filePath("0000-index/tool-backups");
                if (QFileInfo(QFileInfo(folder).absolutePath()).isSymLink() || QFileInfo(QFileInfo(folder).absolutePath()).isJunction() || QFileInfo(folder).isSymLink()
                    || QFileInfo(folder).isJunction() || !QDir().mkpath(folder)) throw QString("Cannot create STEP backup directory");
                const auto backup = QDir(folder).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces) + "-" + QFileInfo(item.path).fileName());
                if (!QFile::copy(item.path, backup)) throw QString("Cannot back up STEP file");
                QSaveFile output(item.path);
                if (!output.open(QIODevice::WriteOnly) || output.write(item.replacement) != item.replacement.size() || !output.commit())
                    throw QString("Cannot save STEP: ") + output.errorString();
                result["backup"] = backup;
            }
            completed.append(result);
            if (onCompleted) onCompleted(item.path);
        } catch (const QString &error) {
            failed.append(QJsonObject{{"path", item.path}, {"reason", error}});
            if (QThread::currentThread()->isInterruptionRequested()) break;
        }
    }
    return {{"schemaVersion", 1}, {"tool", plan.request.tool}, {"preview", false},
        {"completed", completed}, {"failed", failed}, {"skipped", plan.skipped},
        {"cancelled", QThread::currentThread()->isInterruptionRequested()}};
}
