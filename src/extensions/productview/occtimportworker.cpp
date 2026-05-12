#include "occtimportworker.h"

#include <algorithm>
#include <cmath>
#include <exception>

#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <RWStl.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TDF_Label.hxx>
#include <TopoDS_Shape.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

OcctImportWorker::OcctImportWorker(const QString &absolutePath,
                                   FileType::FileType fileType,
                                   quint64 jobId,
                                   QObject *parent)
    : ThreadWorker(parent),
      m_path(absolutePath),
      m_fileType(fileType),
      m_jobId(jobId)
{
    qRegisterMetaType<OcctImportResultPtr>("OcctImportResultPtr");
}

void OcctImportWorker::run()
{
    try
    {
        switch (m_fileType)
        {
        case FileType::STEP:
            importStep();
            break;
        case FileType::IGES:
            importIges();
            break;
        case FileType::STL:
            importStl();
            break;
        default:
            emit failed(m_jobId, tr("Unsupported OCCT preview format."));
            break;
        }
    }
    catch (const Standard_Failure &e)
    {
        const char *message = e.GetMessageString();
        emit failed(m_jobId,
                    message && *message ? QString::fromUtf8(message)
                                        : tr("Unknown %1 import error.").arg(formatName()));
    }
    catch (const std::exception &e)
    {
        emit failed(m_jobId, QString::fromUtf8(e.what()));
    }
    catch (...)
    {
        emit failed(m_jobId, tr("Unknown %1 import error.").arg(formatName()));
    }

    emit finished();
}

QString OcctImportWorker::formatName() const
{
    switch (m_fileType)
    {
    case FileType::STEP:
        return tr("STEP");
    case FileType::IGES:
        return tr("IGES");
    case FileType::STL:
        return tr("STL");
    default:
        return tr("CAD");
    }
}

void OcctImportWorker::importStep()
{
    if (shouldStop())
        return;

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
        emit failed(m_jobId, tr("Failed to read %1 file.").arg(formatName()));
        return;
    }

    if (shouldStop())
        return;

    if (!reader.Transfer(document))
    {
        emit failed(m_jobId,
                    tr("Failed to transfer %1 file into OCCT document.").arg(formatName()));
        return;
    }

    OcctImportResultPtr result;
    if (!collectDocumentShapes(document, formatName(), result))
        return;

    if (shouldStop())
        return;

    meshDocumentShapes(result);

    if (!shouldStop())
        emit imported(m_jobId, result);
}

void OcctImportWorker::importIges()
{
    if (shouldStop())
        return;

    Handle(TDocStd_Document) document;
    Handle(XCAFApp_Application) application = XCAFApp_Application::GetApplication();
    application->NewDocument("MDTV-XCAF", document);

    IGESCAFControl_Reader reader;
    reader.SetColorMode(true);
    reader.SetNameMode(true);
    reader.SetLayerMode(true);

    IFSelect_ReturnStatus status = reader.ReadFile(m_path.toUtf8().constData());
    if (status != IFSelect_RetDone)
    {
        emit failed(m_jobId, tr("Failed to read %1 file.").arg(formatName()));
        return;
    }

    if (shouldStop())
        return;

    if (!reader.Transfer(document))
    {
        emit failed(m_jobId,
                    tr("Failed to transfer %1 file into OCCT document.").arg(formatName()));
        return;
    }

    OcctImportResultPtr result;
    if (!collectDocumentShapes(document, formatName(), result))
        return;

    if (shouldStop())
        return;

    meshDocumentShapes(result);

    if (!shouldStop())
        emit imported(m_jobId, result);
}

void OcctImportWorker::importStl()
{
    if (shouldStop())
        return;

    Handle(Poly_Triangulation) triangulation = RWStl::ReadFile(m_path.toUtf8().constData());
    if (triangulation.IsNull() || !triangulation->HasGeometry())
    {
        emit failed(m_jobId,
                    tr("The %1 file does not contain any displayable shapes.").arg(formatName()));
        return;
    }

    OcctImportResultPtr result(new OcctImportResult());
    result->triangulations.push_back(triangulation);
    addTriangulationBounds(triangulation, result->bbox);

    if (!shouldStop())
        emit imported(m_jobId, result);
}

bool OcctImportWorker::collectDocumentShapes(const Handle(TDocStd_Document) &document,
                                             const QString &formatName,
                                             OcctImportResultPtr &result)
{
    if (shouldStop())
        return false;

    Handle(XCAFDoc_ShapeTool) shapeTool =
            XCAFDoc_DocumentTool::ShapeTool(document->Main());
    TDF_LabelSequence roots;
    shapeTool->GetFreeShapes(roots);

    if (roots.Length() == 0)
    {
        emit failed(m_jobId,
                    tr("The %1 file does not contain any displayable shapes.").arg(formatName));
        return false;
    }

    result.reset(new OcctImportResult());
    result->document = document;
    result->rootLabels = roots;

    for (Standard_Integer i = 1; i <= roots.Length(); ++i)
    {
        if (shouldStop())
            return false;

        TopoDS_Shape shape = shapeTool->GetShape(roots.Value(i));
        if (!shape.IsNull())
            BRepBndLib::Add(shape, result->bbox);
    }

    return true;
}

void OcctImportWorker::meshDocumentShapes(const OcctImportResultPtr &result)
{
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

    Handle(XCAFDoc_ShapeTool) shapeTool =
            XCAFDoc_DocumentTool::ShapeTool(result->document->Main());
    const Standard_Real angularDeflection = 0.5;
    for (Standard_Integer i = 1; i <= result->rootLabels.Length(); ++i)
    {
        if (shouldStop())
            return;

        TopoDS_Shape shape = shapeTool->GetShape(result->rootLabels.Value(i));
        if (!shape.IsNull())
            BRepMesh_IncrementalMesh(shape, linearDeflection, false, angularDeflection, true);
    }
}

void OcctImportWorker::addTriangulationBounds(const Handle(Poly_Triangulation) &triangulation,
                                              Bnd_Box &bbox) const
{
    if (triangulation.IsNull())
        return;

    if (triangulation->HasCachedMinMax())
    {
        bbox.Add(triangulation->CachedMinMax());
        return;
    }

    for (Standard_Integer i = 1; i <= triangulation->NbNodes(); ++i)
        bbox.Add(triangulation->Node(i));
}
