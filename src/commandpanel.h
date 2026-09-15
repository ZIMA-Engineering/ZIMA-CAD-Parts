#ifndef COMMANDPANEL_H
#define COMMANDPANEL_H
#include <QWidget>
#include <QPointer>
#include <QThread>
#include <functional>
#include "core/partscommand.h"
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class CommandPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CommandPanel(std::function<PartsCore::CommandContext()> context, QWidget *parent = nullptr);
    ~CommandPanel() override;
    void focusInput();
signals:
    void commandFinished(int code);
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    void submit();
    void retranslate();
    std::function<PartsCore::CommandContext()> m_context;
    QLineEdit *m_input;
    QPlainTextEdit *m_output;
    QPushButton *m_run;
    QPushButton *m_clear;
    QLabel *m_hint;
    QPointer<QThread> m_worker;
    QStringList m_history;
    int m_historyIndex = 0;
    QString m_draft;
};
#endif
