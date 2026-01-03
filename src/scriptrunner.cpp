#include "scriptrunner.h"
#include "settings.h"

#include <QMessageBox>
#include <QDebug>
#include <QDir>

ScriptRunner::ScriptRunner(const QString &dsPath, QObject *parent) :
    QObject(parent),
    m_dsPath(dsPath)
{

}

void ScriptRunner::run(const QFileInfo &script, const QFileInfo &dir)
{
    auto proc = new QProcess(this);
    proc->setWorkingDirectory(dir.absoluteFilePath());
    proc->setProgram(Settings::get()->TerminalPath);

    proc->setArguments(buildArguments(script, dir));

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("ZCP_WORKDIR", Settings::get()->getWorkingDir());
    env.insert("ZCP_DATASOURCE_ROOT", m_dsPath);
    env.insert("ZCP_DIRECTORY", dir.absoluteFilePath());
    proc->setProcessEnvironment(env);

    connect(
        proc, &QProcess::started,
    [=]() {
        onScriptStarted(script, proc);
    }
    );

    connect(
        proc, &QProcess::errorOccurred,
    [=](QProcess::ProcessError error) {
        onScriptError(script, proc, error);
    }
    );

    connect(
        proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    [=](int exitCode, QProcess::ExitStatus exitStatus) {
        onScriptFinished(script, proc, exitCode, exitStatus);
    }
    );

    proc->startDetached();
}

void ScriptRunner::onScriptStarted(const QFileInfo &script, QProcess *process)
{
    Q_UNUSED(process);

    qDebug() << "Started script" << script;
}

void ScriptRunner::onScriptError(const QFileInfo &script, QProcess *process, QProcess::ProcessError error)
{
    qDebug() << "Script" << script << "failed with error" << error;

    if (error == QProcess::FailedToStart) {
        QMessageBox::warning(
            0,
            tr("Script failed to run"),
            tr("Unable to run script '%1' within terminal '%2': failed to start")
            .arg(script.absoluteFilePath())
            .arg(process->program())
        );

        process->deleteLater();
    }
}

void ScriptRunner::onScriptFinished(const QFileInfo &script, QProcess *process,
                                    int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Script" << script << "finished with exit status" << exitCode << exitStatus;

    if (exitStatus == QProcess::NormalExit && exitCode != 0) {
        QMessageBox::warning(
            0,
            tr("Script failed"),
            tr("Script '%1' failed with exit status %2 with error output:<br><br><pre>%3</pre>")
            .arg(script.absoluteFilePath())
            .arg(exitCode)
            .arg(QString(process->readAllStandardError()))
        );
    }

    process->deleteLater();
}

QStringList ScriptRunner::buildArguments(const QFileInfo &script, const QFileInfo &dir) const
{
#ifdef Q_OS_WIN
    QFileInfo terminalInfo(Settings::get()->TerminalPath);
    const QString terminalExecutable = terminalInfo.fileName();

    if (terminalExecutable.compare(QStringLiteral("cmd.exe"), Qt::CaseInsensitive) == 0) {
        // Keep the window open after the script finishes.
        const QString scriptPath = QDir::toNativeSeparators(script.absoluteFilePath());
        // Pass the raw path and let QProcess handle quoting to avoid double quoting.
        return QStringList() << "/K" << scriptPath;
    }

    const QString cygwinScript = quoteForBash(toCygwinPath(script.absoluteFilePath()));
    const QString cygwinDir = quoteForBash(toCygwinPath(dir.absoluteFilePath()));
    const QString command = QStringLiteral("cd %1 && %2").arg(cygwinDir, cygwinScript);

    if (terminalExecutable.compare(QStringLiteral("mintty.exe"), Qt::CaseInsensitive) == 0) {
        return QStringList() << "-e" << "/bin/bash" << "-lc" << command;
    }

    if (terminalExecutable.contains(QStringLiteral("bash"), Qt::CaseInsensitive)) {
        return QStringList() << "-lc" << command;
    }

    // Fallback to the old behaviour if a custom terminal is configured.
    return QStringList() << "-e" << script.absoluteFilePath();
#else
    Q_UNUSED(dir);
    return QStringList() << "-e" << script.absoluteFilePath();
#endif
}

#ifdef Q_OS_WIN
QString ScriptRunner::toCygwinPath(const QString &path) const
{
    QString normalized = QDir::fromNativeSeparators(path);

    // Drive letter path: C:/foo -> /cygdrive/c/foo
    if (normalized.size() > 1 && normalized[1] == QLatin1Char(':')) {
        const QString driveLetter = normalized.left(1).toLower();
        QString rest = normalized.mid(2);
        if (rest.startsWith(QLatin1Char('/')))
            rest.remove(0, 1);

        return QStringLiteral("/cygdrive/%1/%2").arg(driveLetter, rest);
    }

    return normalized;
}

QString ScriptRunner::quoteForBash(const QString &text) const
{
    QString escaped = text;
    escaped.replace("'", "'\"'\"'");
    return QStringLiteral("'%1'").arg(escaped);
}
#endif
