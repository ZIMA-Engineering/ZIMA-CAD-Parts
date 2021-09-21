#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include <QProcess>
#include <QFileInfo>

class ScriptRunner : public QObject
{
	Q_OBJECT
public:
	explicit ScriptRunner(QObject *parent = nullptr);
	void run(const QFileInfo &script, const QFileInfo &dir);

private slots:
	void onScriptStarted(const QFileInfo &script, QProcess *process);
	void onScriptError(const QFileInfo &script, QProcess *process,
					   QProcess::ProcessError error);
	void onScriptFinished(const QFileInfo &script, QProcess *process,
						  int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // SCRIPTRUNNER_H
