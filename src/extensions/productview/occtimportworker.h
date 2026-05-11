#ifndef OCCTIMPORTWORKER_H
#define OCCTIMPORTWORKER_H

#include "threadworker.h"

#include <QSharedPointer>
#include <QString>
#include <QMetaType>

#include <Bnd_Box.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDocStd_Document.hxx>

struct OcctImportResult
{
    Handle(TDocStd_Document) document;
    TDF_LabelSequence rootLabels;
    Bnd_Box bbox;
};

using OcctImportResultPtr = QSharedPointer<OcctImportResult>;
Q_DECLARE_METATYPE(OcctImportResultPtr)

class OcctImportWorker : public ThreadWorker
{
    Q_OBJECT

public:
    explicit OcctImportWorker(const QString &absolutePath,
                              quint64 jobId,
                              QObject *parent = nullptr);

signals:
    void imported(quint64 jobId, const OcctImportResultPtr &result);
    void failed(quint64 jobId, const QString &message);

public slots:
    void run() override;

private:
    QString m_path;
    quint64 m_jobId;
};

#endif // OCCTIMPORTWORKER_H
