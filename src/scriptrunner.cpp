#include "scriptrunner.h"
#include "settings.h"

#include <QMessageBox>
#include <QDebug>

ScriptRunner::ScriptRunner(QObject *parent) : QObject(parent)
{

}

void ScriptRunner::run(const QFileInfo &script, const QFileInfo &dir)
{
	auto proc = new QProcess(this);
	proc->setWorkingDirectory(dir.absoluteFilePath());
	proc->setProgram(Settings::get()->TerminalPath);

	QStringList args;
	args << "-e" << script.absoluteFilePath();
	proc->setArguments(args);

	auto env = QProcessEnvironment::systemEnvironment();
	env.insert("ZCP_WORKDIR", Settings::get()->getWorkingDir());
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

	proc->start();
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
			tr("Script '%s' failed with exit status %2 with error output:<br><br><pre>%3</pre>")
				.arg(script.absoluteFilePath())
				.arg(exitCode)
				.arg(QString(process->readAllStandardError()))
		);
	}

	process->deleteLater();
}
