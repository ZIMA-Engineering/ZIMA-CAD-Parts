#include "occtimportworker.h"

#include <algorithm>
#include <cmath>
#include <exception>

#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TDF_Label.hxx>
#include <TopoDS_Shape.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

OcctImportWorker::OcctImportWorker(const QString &absolutePath,
                                   quint64 jobId,
                                   QObject *parent)
    : ThreadWorker(parent),
      m_path(absolutePath),
      m_jobId(jobId)
{
    qRegisterMetaType<OcctImportResultPtr>("OcctImportResultPtr");
}

void OcctImportWorker::run()
{
    try
    {
        if (shouldStop())
        {
            emit finished();
            return;
        }

        Handle(TDocStd_Document) document;
        Handle(XCAFApp_Application) application = XCAFApp_Application::GetApplication();
        application->NewDocument("MDTV-XCAF", document);

        STEPCAFControl_Reader reader;
        reader.SetColorMode(true);
        reader.SetNameMode(true);
        reader.SetLayerMode(true);
        reader.SetMatMode(true);

        IFSelect_ReturnStatus status = reader.ReadFile(m_path.toUtf8().constData());
        if (status != IFSelect_RetDone)
        {
            emit failed(m_jobId, tr("Failed to read STEP file."));
            emit finished();
            return;
        }

        if (shouldStop())
        {
            emit finished();
            return;
        }

        if (!reader.Transfer(document))
        {
            emit failed(m_jobId, tr("Failed to transfer STEP file into OCCT document."));
            emit finished();
            return;
        }

        if (shouldStop())
        {
            emit finished();
            return;
        }

        Handle(XCAFDoc_ShapeTool) shapeTool =
                XCAFDoc_DocumentTool::ShapeTool(document->Main());
        TDF_LabelSequence roots;
        shapeTool->GetFreeShapes(roots);

        if (roots.Length() == 0)
        {
            emit failed(m_jobId, tr("The STEP file does not contain any displayable shapes."));
            emit finished();
            return;
        }

        OcctImportResultPtr result(new OcctImportResult());
        result->document = document;
        result->rootLabels = roots;

        for (Standard_Integer i = 1; i <= roots.Length(); ++i)
        {
            if (shouldStop())
            {
                emit finished();
                return;
            }

            TopoDS_Shape shape = shapeTool->GetShape(roots.Value(i));
            if (!shape.IsNull())
                BRepBndLib::Add(shape, result->bbox);
        }

        Standard_Real linearDeflection = 0.1;
        if (!result->bbox.IsVoid())
        {
            Standard_Real xmin = 0.0;
            Standard_Real ymin = 0.0;
            Standard_Real zmin = 0.0;
            Standard_Real xmax = 0.0;
            Standard_Real ymax = 0.0;
            Standard_Real zmax = 0.0;
            result->bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
            const Standard_Real dx = xmax - xmin;
            const Standard_Real dy = ymax - ymin;
            const Standard_Real dz = zmax - zmin;
            const Standard_Real diagonal = std::sqrt(dx * dx + dy * dy + dz * dz);
            linearDeflection = std::max(diagonal * 0.002, 0.01);
        }

        const Standard_Real angularDeflection = 0.5;
        for (Standard_Integer i = 1; i <= roots.Length(); ++i)
        {
            if (shouldStop())
            {
                emit finished();
                return;
            }

            TopoDS_Shape shape = shapeTool->GetShape(roots.Value(i));
            if (!shape.IsNull())
                BRepMesh_IncrementalMesh(shape, linearDeflection, false, angularDeflection, true);
        }

        emit imported(m_jobId, result);
    }
    catch (const Standard_Failure &e)
    {
        const char *message = e.GetMessageString();
        emit failed(m_jobId,
                    message && *message ? QString::fromUtf8(message)
                                        : tr("Unknown STEP import error."));
    }
    catch (const std::exception &e)
    {
        emit failed(m_jobId, QString::fromUtf8(e.what()));
    }
    catch (...)
    {
        emit failed(m_jobId, tr("Unknown STEP import error."));
    }

    emit finished();
}
