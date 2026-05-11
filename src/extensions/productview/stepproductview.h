#ifndef STEPPRODUCTVIEW_H
#define STEPPRODUCTVIEW_H

#include "abstractproductview.h"
#include "stepimportworker.h"

#include <QPointer>

class QThread;

namespace Ui {
class StepProductView;
}

class StepProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit StepProductView(QWidget *parent = nullptr);
    ~StepProductView();

    QString title() override;
    FileTypeList canHandle() override;
    bool handle(FileMetadata *f) override;

protected:
    void hideEvent(QHideEvent *event) override;

private slots:
    void onImported(quint64 jobId, const StepImportResultPtr &result);
    void onFailed(quint64 jobId, const QString &message);

private:
    void cancelActiveWorker();
    void setControlsEnabled(bool enabled);
    void startWorker(const QString &absolutePath);

    Ui::StepProductView *ui;
    quint64 m_jobId;
    QPointer<QThread> m_workerThread;
    QPointer<StepImportWorker> m_worker;
    StepImportResultPtr m_lastResult;
};

#endif // STEPPRODUCTVIEW_H
