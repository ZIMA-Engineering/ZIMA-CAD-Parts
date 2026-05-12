#ifndef OCCTPRODUCTVIEW_H
#define OCCTPRODUCTVIEW_H

#ifdef _MSC_VER
#include <cmath>
#endif

#include "occtimportworker.h"
#include "abstractproductview.h"

#include <QPointer>

class QThread;

namespace Ui {
class OcctProductView;
}

class OcctProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit OcctProductView(QWidget *parent = nullptr);
    ~OcctProductView();

    QString title() override;
    FileTypeList canHandle() override;
    bool handle(FileMetadata *f) override;

protected:
    void hideEvent(QHideEvent *event) override;

private slots:
    void onImported(quint64 jobId, const OcctImportResultPtr &result);
    void onFailed(quint64 jobId, const QString &message);

private:
    void cancelActiveWorker();
    void setControlsEnabled(bool enabled);
    void startWorker(const QString &absolutePath, FileType::FileType fileType);

    Ui::OcctProductView *ui;
    quint64 m_jobId;
    QPointer<QThread> m_workerThread;
    QPointer<OcctImportWorker> m_worker;
    OcctImportResultPtr m_lastResult;
};

#endif // OCCTPRODUCTVIEW_H
