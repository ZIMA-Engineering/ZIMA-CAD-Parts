// SPDX-License-Identifier: GPL-3.0-or-later
#include "systemcommanddialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <signal.h>
#endif

SystemCommandDialog::SystemCommandDialog(const QString &command, const QString &directory,
    const QString &reason, int timeoutSeconds, QWidget *parent)
    : QDialog(parent), m_command(command), m_directory(directory), m_timeoutSeconds(timeoutSeconds)
{
    setObjectName("systemCommandReview");
    setWindowTitle(tr("Review system command"));
    resize(820, 590);
#ifdef Q_OS_WIN
    m_shell = QDir(qEnvironmentVariable("SystemRoot")).filePath("System32/WindowsPowerShell/v1.0/powershell.exe");
#else
    m_shell = "/bin/sh";
#endif
    auto layout = new QVBoxLayout(this);
    const auto label = [this, layout](const QString &text) {
        auto widget = new QLabel(text, this); widget->setTextFormat(Qt::PlainText);
        widget->setWordWrap(true); layout->addWidget(widget); return widget;
    };
    label(tr("Purpose: %1").arg(reason));
    label(tr("Working directory: %1").arg(QDir::toNativeSeparators(directory)));
    label(tr("Shell: %1").arg(QDir::toNativeSeparators(m_shell)));
    auto preview = new QPlainTextEdit(command, this); preview->setObjectName("systemCommandPreview");
    preview->setReadOnly(true); preview->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(preview, 1);
    label(tr("Runs with your user permissions, including access outside the attached paths. "
        "System commands do not enforce Parts directory locks."));
    m_outputView = new QPlainTextEdit(this); m_outputView->setObjectName("systemCommandOutput");
    m_outputView->setReadOnly(true); m_outputView->setFont(preview->font()); m_outputView->setMaximumBlockCount(1000);
    layout->addWidget(m_outputView, 1);
    m_status = label(tr("Nothing runs until you confirm this command."));
    auto buttons = new QHBoxLayout; buttons->addStretch();
    m_run = new QPushButton(tr("Run command"), this); m_run->setObjectName("runSystemCommand");
    m_run->setAutoDefault(false); buttons->addWidget(m_run);
    m_cancel = new QPushButton(tr("Cancel"), this); m_cancel->setDefault(true); buttons->addWidget(m_cancel);
    layout->addLayout(buttons);
    connect(m_run, &QPushButton::clicked, this, &SystemCommandDialog::start);
    connect(m_cancel, &QPushButton::clicked, this, &SystemCommandDialog::reject);
    m_timeout.setSingleShot(true);
    connect(&m_timeout, &QTimer::timeout, this, [this] { m_timedOut = true; stop(); });
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &SystemCommandDialog::readOutput);
    connect(&m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &SystemCommandDialog::complete);
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            m_result = {{"error", tr("Could not start the system shell.")}, {"cancelled", false}};
            complete(-1, QProcess::CrashExit);
        }
    });
    connect(&m_process, &QProcess::started, this, [this] {
        m_processGroup = m_process.processId();
#ifdef Q_OS_WIN
        m_job = CreateJobObjectW(nullptr, nullptr);
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        const auto process = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, DWORD(m_processGroup));
        const bool controlled = m_job && SetInformationJobObject(m_job, JobObjectExtendedLimitInformation, &limits, sizeof(limits))
            && process && AssignProcessToJobObject(m_job, process);
        if (process) CloseHandle(process);
        if (!controlled) {
            m_result["error"] = tr("Could not control the system process tree."); stop(); return;
        }
#endif
        m_process.closeWriteChannel(); // Non-interactive commands only.
        m_timeout.start(m_timeoutSeconds * 1000);
    });
}

SystemCommandDialog::~SystemCommandDialog()
{
    m_process.disconnect(this);
    stop();
    if (m_process.state() != QProcess::NotRunning) m_process.waitForFinished(3000);
#ifdef Q_OS_WIN
    if (m_job) CloseHandle(m_job);
#endif
}

void SystemCommandDialog::setInlineReview()
{
    m_run->setText(tr("Allow"));
    m_cancel->setText(tr("Deny"));
    m_cancel->setObjectName("denySystemCommand");
    m_outputView->hide(); // The result is appended to the command transcript.
    auto preview = findChild<QPlainTextEdit *>("systemCommandPreview");
    preview->setMinimumHeight(80);
    preview->setMaximumHeight(140);
}

void SystemCommandDialog::start()
{
    if (m_started) return;
    if (!QFileInfo(m_directory).isDir() || QFileInfo(m_directory).canonicalFilePath() != m_directory) {
        m_result = {{"error", tr("The working directory changed. Request the command again.")}};
        accept(); return;
    }
    m_started = true;
    m_result = {};
    m_run->setEnabled(false); m_cancel->setText(tr("Stop")); m_status->setText(tr("Command is running..."));
    m_process.setWorkingDirectory(m_directory);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    auto environment = QProcessEnvironment::systemEnvironment();
    for (const auto &key : environment.keys())
        if (key.startsWith("CODEX_") || key.startsWith("OPENAI_") || key.startsWith("OTEL_")) environment.remove(key);
    environment.insert("PYTHONIOENCODING", "utf-8");
    m_process.setProcessEnvironment(environment);
#ifdef Q_OS_WIN
    m_process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) { args->flags |= CREATE_NO_WINDOW; });
    const auto script = QStringLiteral("[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)\n"
        "$OutputEncoding = [Console]::OutputEncoding\n$ErrorActionPreference = 'Stop'\n$global:LASTEXITCODE = 0\n")
        + m_command + "\nif ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }\n";
    const auto encoded = QByteArray(reinterpret_cast<const char *>(script.utf16()), script.size() * 2).toBase64();
    m_process.start(m_shell, {"-NoLogo", "-NoProfile", "-NonInteractive", "-EncodedCommand", QString::fromLatin1(encoded)});
#else
    QProcess::UnixProcessParameters parameters;
    parameters.flags = QProcess::UnixProcessFlag::CreateNewSession;
    m_process.setUnixProcessParameters(parameters);
    m_process.start(m_shell, {"-c", m_command});
#endif
}

void SystemCommandDialog::readOutput()
{
    const auto bytes = m_process.readAllStandardOutput();
    const auto remaining = qMax(0, 48 * 1024 - int(m_output.size()));
    m_output += bytes.left(remaining);
    if (bytes.size() > remaining) m_truncated = true;
    m_outputView->setPlainText(QString::fromUtf8(m_output));
}

void SystemCommandDialog::stop()
{
    if (!m_started || m_done) return;
    m_stopping = true;
#ifdef Q_OS_WIN
    if (m_job) TerminateJobObject(m_job, 130);
#else
    if (m_processGroup > 0) ::kill(-m_processGroup, SIGKILL);
#endif
    if (m_process.state() != QProcess::NotRunning) m_process.kill();
}

void SystemCommandDialog::complete(int exitCode, QProcess::ExitStatus status)
{
    if (m_done) return;
    readOutput(); m_timeout.stop();
#ifdef Q_OS_WIN
    if (m_job) { CloseHandle(m_job); m_job = nullptr; }
#else
    if (m_processGroup > 0) ::kill(-m_processGroup, SIGKILL);
#endif
    m_done = true; m_processGroup = 0;
    m_result["exitCode"] = exitCode;
    m_result["crashed"] = status == QProcess::CrashExit;
    m_result["cancelled"] = m_stopping && !m_timedOut;
    m_result["timedOut"] = m_timedOut;
    m_result["output"] = QString::fromUtf8(m_output);
    m_result["truncated"] = m_truncated;
    accept();
}

void SystemCommandDialog::reject()
{
    if (m_started && !m_done) { stop(); return; }
    QDialog::reject();
}
