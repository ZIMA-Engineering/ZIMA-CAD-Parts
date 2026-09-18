#ifndef PARTSTOOLSDIALOG_H
#define PARTSTOOLSDIALOG_H
#include <QDialog>
#include <QPointer>
#include <QThread>
#include <QMap>
#include "core/partstools.h"
class QLineEdit;
class QCheckBox;
class QTreeWidget;
class QPushButton;
class QLabel;
class QTimer;
class PartsToolsDialog : public QDialog {
    Q_OBJECT
public:
    PartsToolsDialog(const QString &tool, const QString &path, QWidget *parent = nullptr);
    ~PartsToolsDialog() override;
    static QString title(const QString &tool);
    void setPreparedPlan(const PartsCore::ToolPlan &plan);
    QJsonObject operationResult() const { return m_operationResult; }
signals:
    void filesChanged();
protected:
    void reject() override;
private:
    void start(bool apply);
    void invalidate();
    void schedulePreview();
    void finishFileChanges();
    void showPlan();
    QString m_tool, m_sourcePath, m_fileChangesPath;
    QLineEdit *m_path = nullptr, *m_output = nullptr, *m_masks = nullptr;
    QCheckBox *m_recursive, *m_old = nullptr, *m_deleteSources = nullptr;
    QMap<QString, QPair<QCheckBox *, QLineEdit *>> m_fields;
    QTreeWidget *m_files;
    QLabel *m_status;
    QPushButton *m_preview = nullptr, *m_apply, *m_cancel;
    QWidget *m_options;
    QPointer<QThread> m_worker;
    PartsCore::ToolPlan m_plan;
    bool m_prepared = false;
    QTimer *m_autoPreview = nullptr;
    unsigned m_previewRevision = 0;
    QJsonObject m_operationResult;
};
#endif
