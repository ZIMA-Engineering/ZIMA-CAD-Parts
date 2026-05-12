#ifndef OCCTIMPORTWORKER_H
#define OCCTIMPORTWORKER_H

#ifdef _MSC_VER
#include <cmath>
#endif

#include <vector>

#include <Bnd_Box.hxx>
#include <Poly_Triangulation.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDocStd_Document.hxx>

#include <QMetaType>
#include <QSharedPointer>
#include <QString>

#include "file.h"
#include "threadworker.h"

struct OcctImportResult
{
    Handle(TDocStd_Document) document;
    TDF_LabelSequence rootLabels;
    std::vector<Handle(Poly_Triangulation)> triangulations;
    Bnd_Box bbox;
};

using OcctImportResultPtr = QSharedPointer<OcctImportResult>;
Q_DECLARE_METATYPE(OcctImportResultPtr)

class OcctImportWorker : public ThreadWorker
{
    Q_OBJECT

public:
    explicit OcctImportWorker(const QString &absolutePath,
                              FileType::FileType fileType,
                              quint64 jobId,
                              QObject *parent = nullptr);

signals:
    void imported(quint64 jobId, const OcctImportResultPtr &result);
    void failed(quint64 jobId, const QString &message);

public slots:
    void run() override;

private:
    QString formatName() const;
    void importStep();
    void importIges();
    void importStl();
    bool collectDocumentShapes(const Handle(TDocStd_Document) &document,
                               const QString &formatName,
                               OcctImportResultPtr &result);
    void meshDocumentShapes(const OcctImportResultPtr &result);
    void addTriangulationBounds(const Handle(Poly_Triangulation) &triangulation,
                                Bnd_Box &bbox) const;

    QString m_path;
    FileType::FileType m_fileType;
    quint64 m_jobId;
};

#endif // OCCTIMPORTWORKER_H
