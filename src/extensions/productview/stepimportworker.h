#ifndef STEPIMPORTWORKER_H
#define STEPIMPORTWORKER_H

#include "threadworker.h"

#include <QSharedPointer>
#include <QString>
#include <QMetaType>

#include <Bnd_Box.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDocStd_Document.hxx>

struct StepImportResult
{
    Handle(TDocStd_Document) document;
    TDF_LabelSequence rootLabels;
    Bnd_Box bbox;
};

using StepImportResultPtr = QSharedPointer<StepImportResult>;
Q_DECLARE_METATYPE(StepImportResultPtr)

class StepImportWorker : public ThreadWorker
{
    Q_OBJECT

public:
    explicit StepImportWorker(const QString &absolutePath,
                              quint64 jobId,
                              QObject *parent = nullptr);

signals:
    void imported(quint64 jobId, const StepImportResultPtr &result);
    void failed(quint64 jobId, const QString &message);

public slots:
    void run() override;

private:
    QString m_path;
    quint64 m_jobId;
};

#endif // STEPIMPORTWORKER_H
