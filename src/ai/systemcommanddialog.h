// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SYSTEMCOMMANDDIALOG_H
#define SYSTEMCOMMANDDIALOG_H
#include <QDialog>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>
class QLabel;
class QPushButton;
class QPlainTextEdit;

class SystemCommandDialog : public QDialog
{
    Q_OBJECT
public:
    SystemCommandDialog(const QString &command, const QString &directory, const QString &reason,
        int timeoutSeconds, QWidget *parent = nullptr);
    ~SystemCommandDialog() override;
    QJsonObject operationResult() const { return m_result; }
    bool wasStarted() const { return m_started; }
    void setInlineReview();
protected:
    void reject() override;
private:
    void start();
    void stop();
    void readOutput();
    void complete(int exitCode, QProcess::ExitStatus status);
    QString m_command, m_directory, m_shell;
    QProcess m_process;
    QTimer m_timeout;
    int m_timeoutSeconds;
    qint64 m_processGroup = 0;
    void *m_job = nullptr;
    bool m_started = false, m_stopping = false, m_done = false, m_timedOut = false;
    QByteArray m_output;
    bool m_truncated = false;
    QJsonObject m_result{{"cancelled", true}};
    QPlainTextEdit *m_outputView;
    QLabel *m_status;
    QPushButton *m_run, *m_cancel;
};
#endif
