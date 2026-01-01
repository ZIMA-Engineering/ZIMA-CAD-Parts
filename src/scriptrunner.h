#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include <QProcess>
#include <QFileInfo>
#include <QStringList>

class ScriptRunner : public QObject
{
    Q_OBJECT
public:
    explicit ScriptRunner(const QString &dsPath, QObject *parent = nullptr);
    void run(const QFileInfo &script, const QFileInfo &dir);

private:
    QString m_dsPath;
    QStringList buildArguments(const QFileInfo &script, const QFileInfo &dir) const;
#ifdef Q_OS_WIN
    QString toCygwinPath(const QString &path) const;
    QString quoteForBash(const QString &text) const;
    QString quoteForCmd(const QString &text) const;
#endif

private slots:
    void onScriptStarted(const QFileInfo &script, QProcess *process);
    void onScriptError(const QFileInfo &script, QProcess *process,
                       QProcess::ProcessError error);
    void onScriptFinished(const QFileInfo &script, QProcess *process,
                          int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // SCRIPTRUNNER_H
