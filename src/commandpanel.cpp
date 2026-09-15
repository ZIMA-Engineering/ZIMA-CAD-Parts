#include "commandpanel.h"
#include <QLineEdit>
#include <QApplication>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QJsonDocument>
#include <memory>
#include <QTimer>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QScrollArea>
#include "ai/aiprovider.h"
#include "ai/aitools.h"
#include "ai/systemcommanddialog.h"
#include "partstoolsdialog.h"
#include "settings.h"

CommandPanel::CommandPanel(std::function<PartsCore::CommandContext()> context, QWidget *parent, AiProvider *provider)
    : QWidget(parent), m_context(std::move(context)), m_ai(provider ? provider : partsAiProvider())
{
    setObjectName("commandPanel");
    setAcceptDrops(true);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(3);
    m_hint = new QLabel(this);
    m_hint->setWordWrap(true);
    auto header = new QHBoxLayout;
    header->addWidget(m_hint, 1);
    m_aiSettings = new QPushButton(this); m_aiSettings->setObjectName("commandPanelAiSettings");
    header->addWidget(m_aiSettings); layout->addLayout(header);
    m_aiStatus = new QLabel(this); m_aiStatus->setWordWrap(true); m_aiStatus->setTextFormat(Qt::PlainText);
    layout->addWidget(m_aiStatus);
    m_output = new QPlainTextEdit(this);
    m_output->setObjectName("commandPanelOutput");
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(1500);
    m_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_output->installEventFilter(this);
    m_output->viewport()->installEventFilter(this);
    layout->addWidget(m_output, 1);
    auto row = new QHBoxLayout;
    m_input = new QLineEdit(this);
    m_input->setObjectName("commandPanelInput");
    m_input->setFont(m_output->font());
    m_input->installEventFilter(this);
    row->addWidget(m_input, 1);
    m_run = new QPushButton(this);
    m_run->setObjectName("commandPanelRun");
    row->addWidget(m_run);
    m_stop = new QPushButton(this); m_stop->setObjectName("commandPanelStop"); row->addWidget(m_stop);
    m_clear = new QPushButton(this);
    m_clear->setObjectName("commandPanelClear");
    row->addWidget(m_clear);
    layout->addLayout(row);
    connect(m_input, &QLineEdit::returnPressed, this, &CommandPanel::submit);
    connect(m_run, &QPushButton::clicked, this, &CommandPanel::submit);
    connect(m_clear, &QPushButton::clicked, m_output, &QPlainTextEdit::clear);
    connect(m_aiSettings, &QPushButton::clicked, this, &CommandPanel::aiSettingsRequested);
    connect(m_stop, &QPushButton::clicked, this, &CommandPanel::stopAi);
    connect(m_ai, &AiProvider::changed, this, &CommandPanel::retranslate);
    connect(m_ai, &AiProvider::message, this, [this](const QString &text) { if (m_aiPending) appendOutput(text); });
    connect(m_ai, &AiProvider::answer, this, [this] { if (m_aiPending) finishAi(0); });
    connect(m_ai, &AiProvider::toolRequested, this, &CommandPanel::runAiTool);
    connect(m_ai, &AiProvider::failed, this, [this](const QString &error) {
        if (m_aiPending) { appendOutput(tr("Error: %1").arg(error)); finishAi(3); }
    });
    connect(m_ai, &AiProvider::cancelled, this, [this] {
        if (!m_aiPending) return;
        m_aiPending = false;
        if (m_worker) m_worker->requestInterruption();
        if (m_aiReview) m_aiReview->close();
        if (m_systemReview) m_systemReview->close();
        appendOutput(tr("AI stopped. Any completed file operations remain applied."));
        finishAi(130);
    });
    retranslate();
}

CommandPanel::~CommandPanel()
{
    if (m_aiPending) { m_aiPending = false; m_ai->cancel(); }
    if (m_worker) {
        m_worker->requestInterruption();
        m_worker->wait();
    }
}

void CommandPanel::focusInput() { m_input->setFocus(Qt::ShortcutFocusReason); }

bool CommandPanel::addAiReferences(const QStringList &paths)
{
    if (paths.isEmpty()) return false;
    QStringList inserted;
    try {
        for (const auto &value : PartsAi::referenceData(paths)) {
            const auto quoted = PartsAi::quotedPath(value.toObject()["path"].toString());
            if (!m_input->text().contains(quoted)) inserted.append(quoted);
        }
    } catch (const QString &error) {
        appendOutput(tr("Cannot insert AI paths: %1").arg(error)); return false;
    }
    const auto draft = m_input->text();
    const int start = m_input->hasSelectedText() ? m_input->selectionStart() : m_input->cursorPosition();
    const int end = start + m_input->selectedText().size();
    auto text = inserted.join(' ');
    if (!text.isEmpty()) {
        if (start > 0 && !draft[start - 1].isSpace()) text.prepend(' ');
        if (end < draft.size() && !draft[end].isSpace()) text.append(' ');
        if (draft.size() - (end - start) + text.size() > 24000) {
            appendOutput(tr("The AI request is too large.")); return false;
        }
        m_input->insert(text);
    }
    m_aiMode = true; // Inserting text never connects or sends a request by itself.
    retranslate(); focusInput();
    return true;
}

bool CommandPanel::handleReferenceDrop(QDropEvent *event, bool drop)
{
    if (!event->mimeData()->hasUrls()) return false;
    QStringList paths;
    for (const auto &url : event->mimeData()->urls()) {
        if (!url.isLocalFile() || url.toLocalFile().isEmpty()) { event->ignore(); return true; }
        paths.append(url.toLocalFile());
    }
    if (paths.isEmpty() || !(event->possibleActions() & Qt::CopyAction)) { event->ignore(); return true; }
    if (!drop || addAiReferences(paths)) {
        event->setDropAction(Qt::CopyAction); event->accept();
    } else event->ignore();
    return true;
}

void CommandPanel::dragEnterEvent(QDragEnterEvent *event)
{
    if (!handleReferenceDrop(event, false)) event->ignore();
}

void CommandPanel::dropEvent(QDropEvent *event)
{
    if (!handleReferenceDrop(event, true)) event->ignore();
}

void CommandPanel::retranslate()
{
    m_hint->setText(m_aiMode ? tr("Codex conversation. /exit: Parts commands. /new: new conversation. File changes require approval.")
        : tr("Commands: help, list, params, ps2pdf, ptc-clean, step-edit, codex. Tools preview first; --apply executes. History: Up / Down."));
    m_input->setPlaceholderText(m_aiMode ? tr("Ask Codex, or drop files and directories here...")
        : tr("Enter a command, e.g. list or params \"part.pdf\""));
    m_run->setText(m_aiPending ? tr("Working...") : m_worker ? tr("Reading...") : m_aiMode ? tr("Send") : tr("Run"));
    m_run->setEnabled(!m_aiPending && !m_worker && !m_systemReview);
    m_aiSettings->setText(tr("AI settings..."));
    m_aiStatus->setVisible(m_aiMode); m_aiStatus->setText(m_ai->status());
    m_stop->setText(tr("Stop")); m_stop->setVisible(m_aiMode); m_stop->setEnabled(m_aiPending);
    m_clear->setText(tr("Clear output"));
}

void CommandPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) { retranslate(); }
    QWidget::changeEvent(event);
}

bool CommandPanel::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove || event->type() == QEvent::Drop) {
        if (handleReferenceDrop(static_cast<QDropEvent *>(event), event->type() == QEvent::Drop)) return true;
    }
    if (object == m_input && event->type() == QEvent::KeyPress) {
        const auto key = static_cast<QKeyEvent *>(event)->key();
        if (key == Qt::Key_Up && m_historyIndex > 0) {
            if (m_historyIndex == m_history.size()) m_draft = m_input->text();
            m_input->setText(m_history[--m_historyIndex]);
            return true;
        }
        if (key == Qt::Key_Down && m_historyIndex < m_history.size()) {
            ++m_historyIndex;
            m_input->setText(m_historyIndex == m_history.size() ? m_draft : m_history[m_historyIndex]);
            return true;
        }
    }
    return QWidget::eventFilter(object, event);
}

void CommandPanel::submit()
{
    const auto command = m_input->text().trimmed();
    if (command.isEmpty() || m_worker || m_aiPending || m_systemReview) return;
    if (!m_aiMode && command == "codex") {
        m_input->clear(); m_aiMode = true;
        appendOutput(tr("Codex mode. Your requests use the active directory. Type /exit to return."));
        if (!m_ai->ready()) {
            if (!m_ai->busy() && !m_ai->connected()) m_ai->connectAccount(Settings::get()->AiCodexExecutable);
            emit aiSettingsRequested();
        }
        retranslate(); return;
    }
    if (m_aiMode) {
        if (command == "/exit") {
            m_input->clear(); m_aiMode = false; m_aiPlans.clear(); m_aiReferences = {}; m_ai->newConversation(); retranslate(); return;
        }
        if (command == "/new") {
            m_input->clear(); m_ai->newConversation(); m_aiPlans.clear();
            m_aiReferences = {};
            appendOutput(tr("New conversation.")); return;
        }
        submitAi(command); return;
    }
    if (m_history.isEmpty() || m_history.last() != command) m_history.append(command);
    if (m_history.size() > 100) m_history.removeFirst();
    m_historyIndex = m_history.size();
    m_draft.clear();
    m_input->clear();
    const auto context = m_context(); // Capture active tab before the worker starts.
    m_output->appendPlainText(QString("[%1]\n> %2").arg(context.directory, command));
    QStringList arguments;
    try { arguments = PartsCore::splitCommand(command); }
    catch (const QString &error) {
        m_output->appendPlainText(tr("Error: %1").arg(error));
        emit commandFinished(2);
        return;
    }
    auto result = std::make_shared<PartsCore::CommandResult>();
    auto worker = QThread::create([arguments, context, result] {
        *result = PartsCore::executeCommand(arguments, context);
    });
    m_worker = worker;
    m_run->setEnabled(false);
    m_input->setEnabled(false);
    retranslate();
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::finished, this, [this, result] {
        m_worker = nullptr;
        QString text = result->code ? tr("Error: %1").arg(result->error)
            : !result->text.isEmpty() ? result->text
            : QString::fromUtf8(QJsonDocument(result->data).toJson(QJsonDocument::Indented));
        if (result->code && !result->data.isEmpty()) text += '\n' + QString::fromUtf8(QJsonDocument(result->data).toJson(QJsonDocument::Indented));
        if (text.size() > 24000) text = text.left(24000) + '\n' + tr("Output was truncated. Use the CLI for complete JSON.");
        m_output->appendPlainText(text);
        m_run->setEnabled(true);
        m_input->setEnabled(true);
        retranslate();
        if (isAncestorOf(QApplication::focusWidget())) focusInput();
        emit commandFinished(result->code);
        if (!result->code && result->data["requestApplicationClose"].toBool())
            QTimer::singleShot(0, qApp, [] { QApplication::closeAllWindows(); });
    });
    worker->start();
}

void CommandPanel::appendOutput(const QString &text)
{
    m_output->appendPlainText(text.size() > 24000 ? text.left(24000) + '\n' + tr("Output was truncated. Use the CLI for complete JSON.") : text);
}

void CommandPanel::submitAi(const QString &text)
{
    if (!m_ai->ready() || m_ai->busy()) {
        appendOutput(tr("Connect your ChatGPT account in AI settings first."));
        emit aiSettingsRequested(); return;
    }
    if (text.size() > 24000) { appendOutput(tr("The AI request is too large.")); return; }
    const auto nextContext = m_context();
    if (nextContext.directory != m_aiContext.directory || nextContext.workingDirectory != m_aiContext.workingDirectory)
    {
        m_aiPlans.clear(); m_aiReferences = {};
    }
    m_aiContext = nextContext;
    m_aiPending = true;
    m_input->clear(); // Keep the editor available for the next draft; submit() guards the active request.
    appendOutput(QString("[%1]\nCodex > %2").arg(m_aiContext.directory, text));
    auto context = std::make_shared<QJsonObject>();
    auto error = std::make_shared<QString>();
    const auto captured = m_aiContext;
    const auto model = Settings::get()->AiModel;
    const auto previousReferences = m_aiReferences;
    auto worker = QThread::create([context, error, captured, text, previousReferences] {
        try {
            auto references = previousReferences;
            for (const auto &reference : PartsAi::promptReferences(text))
                if (!references.contains(reference)) references.append(reference);
            if (references.size() > 32) throw QString("Too many paths in this conversation. Use /new to start another.");
            *context = PartsAi::contextData(captured, references);
        } catch (const QString &value) { *error = value; }
    });
    m_worker = worker; retranslate();
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::finished, this, [this, context, error, text, model] {
        m_worker = nullptr;
        if (!m_aiPending) { retranslate(); return; }
        if (!error->isEmpty()) { appendOutput(tr("Error: %1").arg(*error)); finishAi(3); return; }
        const auto references = (*context)["references"].toArray();
        if (m_aiReferences != references) m_aiPlans.clear();
        m_aiReferences = references;
        m_ai->ask(text, *context, model);
    });
    worker->start();
}

void CommandPanel::finishAi(int code)
{
    m_aiPending = false;
    if (m_aiReview) m_aiReview->close();
    if (m_systemReview) m_systemReview->close();
    if (code) m_aiPlans.clear();
    m_input->setEnabled(true); retranslate(); emit commandFinished(code);
}

void CommandPanel::stopAi()
{
    if (!m_aiPending) return;
    m_ai->cancel();
}

void CommandPanel::showAiReview(QDialog *review)
{
    // Reuse the exact operation preview, but keep approval inside the command panel.
    // The buttons stay visible while long commands and file lists scroll above them.
    review->setWindowFlags(Qt::Widget);
    review->setModal(false);
    auto reviewLayout = qobject_cast<QVBoxLayout *>(review->layout());
    auto buttons = reviewLayout->takeAt(reviewLayout->count() - 1);
    auto body = new QWidget(review);
    auto bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    while (auto item = reviewLayout->takeAt(0)) {
        if (auto widget = item->widget()) {
            bodyLayout->addWidget(widget); // Reparent widgets into the scroll body.
            delete item;
        } else bodyLayout->addItem(item);
    }
    auto scroll = new QScrollArea(review);
    scroll->setWidgetResizable(true);
    scroll->setWidget(body);
    scroll->setFrameShape(QFrame::NoFrame);
    reviewLayout->addWidget(scroll, 1);
    reviewLayout->addItem(buttons);
    reviewLayout->setContentsMargins(4, 4, 4, 4);
    review->setMaximumHeight(360);
    auto panelLayout = qobject_cast<QVBoxLayout *>(layout());
    panelLayout->insertWidget(panelLayout->indexOf(m_output) + 1, review, 2);
    review->show();
}

void CommandPanel::runAiTool(const QString &id, const QString &name, const QJsonObject &arguments)
{
    if (!m_aiPending) return;
    if (m_worker || m_aiReview || m_systemReview) {
        m_ai->toolResult(id, {{"error", "Another local operation is in progress. Call tools sequentially."}}, false); return;
    }
    if (name == "system_command") {
        QString directory;
        const auto command = arguments["command"].toString();
        const auto reason = arguments["reason"].toString();
        const auto timeout = arguments["timeoutSeconds"].toInt(-1);
        try {
            if (arguments.size() != 4 || !arguments["directory"].isString() || command.trimmed().isEmpty()
                || command.size() > 24000 || command.contains(QChar(0)) || reason.isEmpty() || reason.size() > 2000
                || timeout < 1 || timeout > 600 || arguments["timeoutSeconds"].toDouble() != timeout)
                throw QString("Expected command, directory, reason and timeoutSeconds (1..600)");
            directory = PartsAi::checkedPath(arguments["directory"].toString(), m_aiContext, m_aiReferences);
            if (!QFileInfo(directory).isDir()) throw QString("Expected a working directory");
        } catch (const QString &error) {
            m_ai->toolResult(id, {{"error", error}}, false); return;
        }
        auto dialog = new SystemCommandDialog(command, directory, reason, timeout, this);
        dialog->setInlineReview();
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        m_systemReview = dialog;
        connect(dialog, &QDialog::finished, this, [this, dialog, directory, command, id] {
            const auto result = dialog->operationResult();
            m_systemReview = nullptr;
            appendOutput(tr("System command: %1").arg(command));
            appendOutput(QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Indented)));
            if (dialog->wasStarted()) {
                emit filesChanged(directory);
                emit filesChanged(m_aiContext.directory);
                if (!m_aiContext.workingDirectory.isEmpty()) emit filesChanged(m_aiContext.workingDirectory);
                for (const auto &value : m_aiReferences) {
                    const QFileInfo file(value.toObject()["path"].toString());
                    emit filesChanged(file.isDir() ? file.absoluteFilePath() : file.absolutePath());
                }
            }
            const bool success = !result.contains("error") && result["exitCode"].toInt(-1) == 0
                && !result["crashed"].toBool() && !result["cancelled"].toBool() && !result["timedOut"].toBool();
            if (m_aiPending) m_ai->toolResult(id, result, success);
            retranslate();
        });
        showAiReview(dialog);
        return;
    }
    if (name == "parts_apply") {
        const auto key = arguments["planId"].toString();
        if (arguments.size() != 1 || !m_aiPlans.contains(key)) {
            m_ai->toolResult(id, {{"error", "Unknown or expired plan. Preview again."}}, false); return;
        }
        const auto plan = m_aiPlans.take(key); // One review per token; never replay approval.
        auto dialog = new PartsToolsDialog(plan.request.tool, plan.request.path, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setPreparedPlan(plan);
        m_aiReview = dialog;
        connect(dialog, &PartsToolsDialog::filesChanged, this, [this, plan] {
            emit filesChanged(QFileInfo(plan.request.path).isDir() ? plan.request.path : QFileInfo(plan.request.path).absolutePath());
        });
        connect(dialog, &QDialog::finished, this, [this, dialog, id](int accepted) {
            const auto result = accepted == QDialog::Accepted ? dialog->operationResult() : QJsonObject{{"cancelled", true}};
            m_aiReview = nullptr;
            const bool success = accepted == QDialog::Accepted && !result.contains("error")
                && result["failed"].toArray().isEmpty() && !result["cancelled"].toBool();
            appendOutput(QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Indented)));
            if (m_aiPending) m_ai->toolResult(id, result, success);
        });
        showAiReview(dialog);
        return;
    }
    appendOutput(tr("AI tool: %1").arg(name));
    auto result = std::make_shared<PartsAi::ToolResult>();
    const auto captured = m_aiContext;
    const auto references = m_aiReferences;
    auto worker = QThread::create([result, captured, references, name, arguments] { *result = PartsAi::runTool(name, arguments, captured, references); });
    m_worker = worker;
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::finished, this, [this, result, id] {
        m_worker = nullptr;
        if (!m_aiPending) { retranslate(); return; }
        if (result->success && !result->plan.request.tool.isEmpty() && !result->plan.items.isEmpty()
            && (result->plan.request.tool != "step-edit" || !result->plan.request.fields.isEmpty())) {
            if (m_aiPlans.size() >= 8) { result->success = false; result->data = {{"error", "Too many pending plans; apply a preview first."}}; }
            else {
                const auto key = QUuid::createUuid().toString(QUuid::WithoutBraces);
                m_aiPlans[key] = result->plan;
                result->data["planId"] = key;
            }
        }
        m_ai->toolResult(id, result->data, result->success);
    });
    worker->start();
}
