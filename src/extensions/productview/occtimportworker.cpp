#include "occtimportworker.h"

#include <algorithm>
#include <cmath>
#include <exception>

#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <RWStl.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TDF_Label.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <QFile>
#include <QFileInfo>

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
        const QFileInfo fileInfo(m_path);
        if (!fileInfo.isFile() || !fileInfo.isReadable())
        {
            emit failed(m_jobId, tr("The CAD file does not exist or is not readable."));
            emit finished();
            return;
        }

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

    emit statusChanged(m_jobId, tr("Reading %1 file...").arg(formatName()));
    const QByteArray encodedPath = QFile::encodeName(m_path);
    IFSelect_ReturnStatus status = reader.ReadFile(encodedPath.constData());
    if (status != IFSelect_RetDone)
    {
        emit failed(m_jobId, tr("Failed to read %1 file.").arg(formatName()));
        return;
    }

    if (shouldStop())
        return;

    emit statusChanged(m_jobId, tr("Converting %1 geometry...").arg(formatName()));
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

    emit statusChanged(m_jobId, tr("Preparing %1 preview...").arg(formatName()));
    meshDocumentShapes(result);

    if (!shouldStop() && result->vertices.isEmpty())
    {
        emit failed(m_jobId,
                    tr("The %1 file does not contain any displayable shapes.").arg(formatName()));
    }
    else if (!shouldStop())
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

    emit statusChanged(m_jobId, tr("Reading %1 file...").arg(formatName()));
    const QByteArray encodedPath = QFile::encodeName(m_path);
    IFSelect_ReturnStatus status = reader.ReadFile(encodedPath.constData());
    if (status != IFSelect_RetDone)
    {
        emit failed(m_jobId, tr("Failed to read %1 file.").arg(formatName()));
        return;
    }

    if (shouldStop())
        return;

    emit statusChanged(m_jobId, tr("Converting %1 geometry...").arg(formatName()));
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

    emit statusChanged(m_jobId, tr("Preparing %1 preview...").arg(formatName()));
    meshDocumentShapes(result);

    if (!shouldStop() && result->vertices.isEmpty())
    {
        emit failed(m_jobId,
                    tr("The %1 file does not contain any displayable shapes.").arg(formatName()));
    }
    else if (!shouldStop())
        emit imported(m_jobId, result);
}

void OcctImportWorker::importStl()
{
    if (shouldStop())
        return;

    emit statusChanged(m_jobId, tr("Reading %1 file...").arg(formatName()));
    const QByteArray encodedPath = QFile::encodeName(m_path);
    Handle(Poly_Triangulation) triangulation = RWStl::ReadFile(encodedPath.constData());
    if (triangulation.IsNull() || !triangulation->HasGeometry())
    {
        emit failed(m_jobId,
                    tr("The %1 file does not contain any displayable shapes.").arg(formatName()));
        return;
    }

    OcctImportResultPtr result(new OcctImportResult());
    result->triangulations.push_back(triangulation);
    addTriangulationBounds(triangulation, result->bbox);
    appendTriangulation(triangulation, TopLoc_Location(), false, result);

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

    for (Standard_Integer i = 1; i <= roots.Length(); ++i)
    {
        if (shouldStop())
            return false;

        TopoDS_Shape shape = shapeTool->GetShape(roots.Value(i));
        if (!shape.IsNull())
        {
            result->rootLabels.Append(roots.Value(i));
            BRepBndLib::Add(shape, result->bbox);
        }
    }

    if (result->rootLabels.Length() == 0 || result->bbox.IsVoid())
    {
        emit failed(m_jobId,
                    tr("The %1 file does not contain any displayable shapes.").arg(formatName));
        result.clear();
        return false;
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
        {
            BRepMesh_IncrementalMesh(shape, linearDeflection, false, angularDeflection, true);

            for (TopExp_Explorer explorer(shape, TopAbs_FACE);
                 explorer.More(); explorer.Next())
            {
                const TopoDS_Face face = TopoDS::Face(explorer.Current());
                TopLoc_Location location;
                const Handle(Poly_Triangulation) triangulation =
                        BRep_Tool::Triangulation(face, location);
                appendTriangulation(triangulation,
                                    location,
                                    face.Orientation() == TopAbs_REVERSED,
                                    result);
            }
        }
    }
}

void OcctImportWorker::appendTriangulation(
        const Handle(Poly_Triangulation) &triangulation,
        const TopLoc_Location &location,
        bool reversed,
        const OcctImportResultPtr &result) const
{
    if (triangulation.IsNull())
        return;

    if (!triangulation->HasNormals())
        triangulation->ComputeNormals();

    const gp_Trsf transformation = location.Transformation();
    for (Standard_Integer i = 1; i <= triangulation->NbTriangles(); ++i)
    {
        Standard_Integer n1 = 0;
        Standard_Integer n2 = 0;
        Standard_Integer n3 = 0;
        triangulation->Triangle(i).Get(n1, n2, n3);
        if (reversed)
            std::swap(n2, n3);

        const gp_Pnt p1 = triangulation->Node(n1).Transformed(transformation);
        const gp_Pnt p2 = triangulation->Node(n2).Transformed(transformation);
        const gp_Pnt p3 = triangulation->Node(n3).Transformed(transformation);
        const QVector3D v1(p1.X(), p1.Y(), p1.Z());
        const QVector3D v2(p2.X(), p2.Y(), p2.Z());
        const QVector3D v3(p3.X(), p3.Y(), p3.Z());
        const QVector3D faceNormal = QVector3D::crossProduct(v2 - v1, v3 - v1);
        if (faceNormal.lengthSquared() <= 0.0f)
            continue;

        const auto transformedNormal = [&](Standard_Integer nodeIndex) {
            gp_Dir normal = triangulation->Normal(nodeIndex);
            normal.Transform(transformation);
            QVector3D result(normal.X(), normal.Y(), normal.Z());
            if (reversed)
                result = -result;
            return result.normalized();
        };

        result->vertices << v1 << v2 << v3;
        result->normals << transformedNormal(n1)
                        << transformedNormal(n2)
                        << transformedNormal(n3);
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
