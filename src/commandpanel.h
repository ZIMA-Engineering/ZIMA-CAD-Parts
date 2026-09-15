#ifndef COMMANDPANEL_H
#define COMMANDPANEL_H
#include <QWidget>
#include <QPointer>
#include <QThread>
#include <functional>
#include "core/partscommand.h"
#include "core/partstools.h"
#include <QHash>
class AiProvider;
class PartsToolsDialog;
class SystemCommandDialog;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class QScrollArea;
class QHBoxLayout;
class QDragEnterEvent;
class QDropEvent;
class QDialog;
class CommandPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CommandPanel(std::function<PartsCore::CommandContext()> context, QWidget *parent = nullptr, AiProvider *provider = nullptr);
    ~CommandPanel() override;
    void focusInput();
    bool addAiReferences(const QStringList &paths);
signals:
    void commandFinished(int code);
    void aiSettingsRequested();
    void filesChanged(const QString &directory);
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private:
    void submit();
    void retranslate();
    void submitAi(const QString &text);
    void runAiTool(const QString &id, const QString &name, const QJsonObject &arguments);
    void stopAi();
    void finishAi(int code);
    void showAiReview(QDialog *review);
    void appendOutput(const QString &text);
    bool handleReferenceDrop(QDropEvent *event, bool drop);
    std::function<PartsCore::CommandContext()> m_context;
    QLineEdit *m_input;
    QPlainTextEdit *m_output;
    QPushButton *m_run;
    QPushButton *m_clear;
    QLabel *m_hint;
    QLabel *m_aiStatus;
    QPushButton *m_aiSettings, *m_stop;
    AiProvider *m_ai;
    bool m_aiMode = false, m_aiPending = false;
    PartsCore::CommandContext m_aiContext;
    QHash<QString, PartsCore::ToolPlan> m_aiPlans;
    QPointer<PartsToolsDialog> m_aiReview;
    QPointer<SystemCommandDialog> m_systemReview;
    QPointer<QThread> m_worker;
    QJsonArray m_aiReferences;
    QStringList m_history;
    int m_historyIndex = 0;
    QString m_draft;
};
#endif
