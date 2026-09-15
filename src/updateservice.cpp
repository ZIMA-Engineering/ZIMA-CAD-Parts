#include "updateservice.h"
#include "update/installationclient.h"
#include "settings.h"
#include "zima-cad-parts.h"
#include <QApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QSettings>
#include <QFile>
#include <QUuid>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QTimer>

UpdateService *UpdateService::get()
{
    static auto service = new UpdateService(qApp);
    return service;
}
UpdateService::UpdateService(QObject *parent) : QObject(parent)
{
    m_progressTimer.start();
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
        m_installAfterDownload = false;
        if (m_process.state() != QProcess::NotRunning) { m_process.kill(); m_process.waitForFinished(1000); }
    });
#ifdef Q_OS_WIN
    m_process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) { args->flags |= CREATE_NO_WINDOW; });
#endif
    QFile previousResult(partsInstallationRoot() + "/.updates/" +
#ifdef Q_OS_WIN
        "windows-x64"
#else
        "debian-13-x86_64"
#endif
        + "-result.json");
    if (!partsInstallationRoot().isEmpty() && previousResult.open(QIODevice::ReadOnly) && previousResult.size() < 65536) {
        const auto result = QJsonDocument::fromJson(previousResult.readAll()).object();
        if (result["status"] == "error") m_previousInstallationError = result["error"].toString();
    }
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this] {
        m_buffer += m_process.readAllStandardOutput();
        if (m_buffer.size() > 4 * 1024 * 1024) { m_process.kill(); m_error = tr("Update response is too large."); return; }
        while (m_buffer.contains('\n')) {
            const auto index = m_buffer.indexOf('\n');
            const auto object = QJsonDocument::fromJson(m_buffer.left(index)).object();
            m_buffer.remove(0, index + 1);
            if (object["event"] == "progress" && object.contains("received")) {
                m_received = object["received"].toInteger(); m_total = object["total"].toInteger();
                if (m_progressTimer.elapsed() >= 100) { m_progressTimer.restart(); emit changed(); }
            }
            if (object["event"] == "progress" && m_phase != object["phase"].toString()) {
                m_phase = object["phase"].toString(); emit changed();
            }
            if (object["event"] == "result") { m_result = object; emit changed(); }
        }
    });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) return;
        m_busy = false; m_installAfterDownload = false;
        m_error = tr("The update component could not be started."); emit changed();
    });
    connect(&m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus exit) {
        m_busy = false;
        if (m_cancelled) { m_error = tr("Update cancelled. The installed version is unchanged."); m_installAfterDownload = false; }
        else if (code || exit != QProcess::NormalExit || m_result.isEmpty()) {
            m_error = m_result["error"].toString();
            if (m_error.isEmpty()) m_error = tr("The update operation failed.");
            m_installAfterDownload = false;
            if (m_command == "check") m_offer = {};
        } else if (m_command == "check") {
            m_offer = m_result["status"] == "available" ? m_result : QJsonObject();
            m_phase = m_result["status"].toString();
        } else if (m_command == "download" && m_installAfterDownload && m_result["status"] == "prepared") {
            m_installAfterDownload = false;
            handoff(false);
        }
        emit changed();
    });
}
void UpdateService::run(const QString &command, const QStringList &arguments)
{
    if (m_busy) return;
    m_received = 0; m_total = 0;
    m_buffer.clear(); m_result = {}; m_error.clear(); m_cancelled = false;
    m_command = command; m_phase = command == "check" ? "checking" : "downloading"; m_busy = true;
    auto args = QStringList{command} + arguments;
    const auto root = partsInstallationRoot();
    if (!root.isEmpty()) args << "--root" << root;
    m_process.start(partsUpdateExecutable(), args);
    emit changed();
}
void UpdateService::check() { if (!m_busy) run("check"); }
void UpdateService::install()
{
    if (m_busy || !m_offer["installable"].toBool()) return;
    m_previousInstallationError.clear();
    m_installAfterDownload = true;
    run("download", {"--target", m_offer["availableVersion"].toString()});
}
void UpdateService::handoff(bool rollback)
{
    const auto root = partsInstallationRoot();
    if (root.isEmpty()) { m_error = tr("Use the distribution launcher to install updates."); return; }
    QStringList args{rollback ? "rollback" : "install", "--root", root, "--apply"};
    if (!rollback) args << "--target" << m_result["version"].toString();
    m_error.clear(); m_previousInstallationError.clear();
    const auto operation = QUuid::createUuid().toString(QUuid::WithoutBraces);
    args << "--operation" << operation;
    QProcess installer;
    installer.setProgram(partsUpdateExecutable()); installer.setArguments(args);
#ifdef Q_OS_WIN
    installer.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) { args->flags |= CREATE_NO_WINDOW; });
#endif
    if (!installer.startDetached()) { m_error = tr("The installer could not be started."); emit changed(); return; }
    m_phase = "waiting"; m_busy = true;
    auto poll = new QTimer(this);
    connect(poll, &QTimer::timeout, this, [this, root, operation, poll] {
        QFile file(root + "/.updates/" +
#ifdef Q_OS_WIN
            "windows-x64"
#else
            "debian-13-x86_64"
#endif
            + "-result.json");
        if (!file.open(QIODevice::ReadOnly) || file.size() > 65536) return;
        const auto result = QJsonDocument::fromJson(file.readAll()).object();
        if (result["operation"] != operation) return;
        m_busy = false; m_error = result["error"].toString(); m_phase = result["status"].toString();
        poll->deleteLater(); emit changed();
    });
    poll->start(1000);
    emit changed();
    QTimer::singleShot(0, qApp, [] { QApplication::closeAllWindows(); });
}
void UpdateService::rollback() { if (!m_busy) handoff(true); }
void UpdateService::cancel()
{
    m_installAfterDownload = false; m_cancelled = true;
    if (m_process.state() != QProcess::NotRunning) m_process.kill();
}
void UpdateService::scheduleStartupCheck()
{
    QTimer::singleShot(1500, this, [this] { if (Settings::get()->UpdatesAutomatic) check(); });
}
void UpdateService::acknowledgeStartup()
{
    const auto name = qEnvironmentVariable("ZCP_UPDATE_SOCKET");
    const auto token = qEnvironmentVariable("ZCP_UPDATE_TOKEN");
    if (name.isEmpty() || token.isEmpty()) return;
    qunsetenv("ZCP_UPDATE_SOCKET"); qunsetenv("ZCP_UPDATE_TOKEN");
    auto socket = new QLocalSocket(this);
    QTimer::singleShot(5000, socket, &QObject::deleteLater);
    connect(socket, &QLocalSocket::connected, this, [socket, token] {
        socket->write(QJsonDocument(QJsonObject{{"token", token}, {"version", VERSION},
            {"pid", QCoreApplication::applicationPid()}}).toJson(QJsonDocument::Compact) + '\n');
        socket->flush();
    });
    connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
    socket->connectToServer(name);
}
