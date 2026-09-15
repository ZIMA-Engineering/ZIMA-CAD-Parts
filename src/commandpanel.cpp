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

CommandPanel::CommandPanel(std::function<PartsCore::CommandContext()> context, QWidget *parent)
    : QWidget(parent), m_context(std::move(context))
{
    setObjectName("commandPanel");
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(3);
    m_hint = new QLabel(this);
    m_hint->setWordWrap(true);
    layout->addWidget(m_hint);
    m_output = new QPlainTextEdit(this);
    m_output->setObjectName("commandPanelOutput");
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(1500);
    m_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
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
    m_clear = new QPushButton(this);
    m_clear->setObjectName("commandPanelClear");
    row->addWidget(m_clear);
    layout->addLayout(row);
    connect(m_input, &QLineEdit::returnPressed, this, &CommandPanel::submit);
    connect(m_run, &QPushButton::clicked, this, &CommandPanel::submit);
    connect(m_clear, &QPushButton::clicked, m_output, &QPlainTextEdit::clear);
    retranslate();
}

CommandPanel::~CommandPanel()
{
    if (m_worker) {
        m_worker->requestInterruption();
        m_worker->wait();
    }
}

void CommandPanel::focusInput() { m_input->setFocus(Qt::ShortcutFocusReason); }

void CommandPanel::retranslate()
{
    m_hint->setText(tr("Commands: help, list, params, ps2pdf, ptc-clean, step-edit. Tools preview first; --apply executes. History: Up / Down."));
    m_input->setPlaceholderText(tr("Enter a command, e.g. list or params \"part.pdf\""));
    m_run->setText(m_worker ? tr("Reading...") : tr("Run"));
    m_clear->setText(tr("Clear output"));
}

void CommandPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslate();
    QWidget::changeEvent(event);
}

bool CommandPanel::eventFilter(QObject *object, QEvent *event)
{
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
    if (command.isEmpty() || m_worker) return;
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
