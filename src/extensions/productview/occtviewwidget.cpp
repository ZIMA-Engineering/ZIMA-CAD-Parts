#include "occtviewwidget.h"

#include <algorithm>
#include <cmath>

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QtMath>
#include <QWheelEvent>

namespace {

const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
uniform mat4 modelViewProjection;
uniform mat3 normalMatrix;
out vec3 surfaceNormal;
void main()
{
    surfaceNormal = normalize(normalMatrix * normal);
    gl_Position = modelViewProjection * vec4(position, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 330 core
in vec3 surfaceNormal;
uniform vec3 baseColor;
out vec4 fragmentColor;
void main()
{
    vec3 lightDirection = normalize(vec3(0.4, 0.6, 1.0));
    float diffuse = max(dot(normalize(surfaceNormal), lightDirection), 0.0);
    float lighting = 0.25 + 0.75 * diffuse;
    fragmentColor = vec4(baseColor * lighting, 1.0);
}
)";

}

OcctViewWidget::OcctViewWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_vertexBuffer(QOpenGLBuffer::VertexBuffer),
      m_rotation(QQuaternion::fromEulerAngles(-25.0f, 35.0f, 0.0f)),
      m_radius(1.0f),
      m_distance(3.0f),
      m_vertexCount(0),
      m_initialized(false),
      m_wireframe(false)
{
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setVersion(3, 3);
    surfaceFormat.setDepthBufferSize(24);
    surfaceFormat.setStencilBufferSize(8);
    surfaceFormat.setSamples(4);
    setFormat(surfaceFormat);
    setFocusPolicy(Qt::StrongFocus);
}

OcctViewWidget::~OcctViewWidget()
{
    if (!m_initialized)
        return;

    makeCurrent();
    m_vertexArray.destroy();
    m_vertexBuffer.destroy();
    doneCurrent();
}

QSize OcctViewWidget::minimumSizeHint() const
{
    return QSize(320, 240);
}

QSize OcctViewWidget::sizeHint() const
{
    return QSize(900, 640);
}

void OcctViewWidget::clearScene()
{
    m_result.clear();
    m_vertexCount = 0;
    if (m_initialized)
    {
        makeCurrent();
        m_vertexBuffer.bind();
        m_vertexBuffer.allocate(nullptr, 0);
        m_vertexBuffer.release();
        doneCurrent();
    }
    update();
}

void OcctViewWidget::setImportedModel(const OcctImportResultPtr &result)
{
    m_result = result;
    fitAll();
    if (m_initialized)
        uploadModel();
}

void OcctViewWidget::fitAll()
{
    if (m_result.isNull() || m_result->bbox.IsVoid())
        return;

    Standard_Real xmin = 0.0;
    Standard_Real ymin = 0.0;
    Standard_Real zmin = 0.0;
    Standard_Real xmax = 0.0;
    Standard_Real ymax = 0.0;
    Standard_Real zmax = 0.0;
    m_result->bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    m_center = QVector3D((xmin + xmax) * 0.5,
                         (ymin + ymax) * 0.5,
                         (zmin + zmax) * 0.5);
    const QVector3D extent(xmax - xmin, ymax - ymin, zmax - zmin);
    m_radius = std::max(extent.length() * 0.5f, 0.001f);
    m_distance = m_radius * 3.0f;
    m_pan = QVector3D();
    updateProjection();
    update();
}

void OcctViewWidget::setIsometricView()
{
    setViewRotation(QQuaternion::fromEulerAngles(-35.264f, 45.0f, 0.0f));
}

void OcctViewWidget::setFrontView()
{
    setViewRotation(QQuaternion());
}

void OcctViewWidget::setTopView()
{
    setViewRotation(QQuaternion::fromEulerAngles(-90.0f, 0.0f, 0.0f));
}

void OcctViewWidget::setRightView()
{
    setViewRotation(QQuaternion::fromEulerAngles(0.0f, -90.0f, 0.0f));
}

void OcctViewWidget::setShadedMode()
{
    m_wireframe = false;
    update();
}

void OcctViewWidget::setWireframeMode()
{
    m_wireframe = true;
    update();
}

bool OcctViewWidget::hasLoadedModel() const
{
    return m_vertexCount > 0;
}

void OcctViewWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.12f, 0.14f, 0.17f, 1.0f);

    if (!m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)
            || !m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)
            || !m_program.link())
    {
        emit initializationFailed(tr("The 3D viewer could not initialize OpenGL."));
        return;
    }

    m_vertexArray.create();
    m_vertexBuffer.create();
    m_initialized = true;
    updateProjection();
    uploadModel();
}

void OcctViewWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!m_initialized || m_vertexCount == 0)
        return;

    QMatrix4x4 model;
    model.rotate(m_rotation);
    model.translate(-m_center);
    QMatrix4x4 view;
    view.translate(m_pan.x(), m_pan.y(), -m_distance);
    const QMatrix4x4 modelView = view * model;

    m_program.bind();
    m_program.setUniformValue("modelViewProjection", m_projection * modelView);
    m_program.setUniformValue("normalMatrix", modelView.normalMatrix());
    m_program.setUniformValue("baseColor", QVector3D(0.18f, 0.65f, 0.88f));
    m_vertexArray.bind();

    glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    m_vertexArray.release();
    m_program.release();
}

void OcctViewWidget::resizeGL(int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    updateProjection();
}

void OcctViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton)
    {
        m_lastMousePosition = event->position().toPoint();
        event->accept();
        return;
    }

    QOpenGLWidget::mousePressEvent(event);
}

void OcctViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::MiddleButton))
    {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }

    const QPoint delta = event->position().toPoint() - m_lastMousePosition;
    m_lastMousePosition = event->position().toPoint();

    if (event->buttons().testFlag(Qt::RightButton))
    {
        const float viewportHeight = std::max(height(), 1);
        const float unitsPerPixel = 2.0f * m_distance
                * std::tan(qDegreesToRadians(22.5f)) / viewportHeight;
        m_pan += QVector3D(delta.x() * unitsPerPixel,
                           -delta.y() * unitsPerPixel,
                           0.0f);
        update();
        event->accept();
        return;
    }

    const QQuaternion horizontal = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f,
                                                                 delta.x() * 0.5f);
    const QQuaternion vertical = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f,
                                                               delta.y() * 0.5f);
    m_rotation = horizontal * vertical * m_rotation;
    update();
    event->accept();
}

void OcctViewWidget::wheelEvent(QWheelEvent *event)
{
    const float steps = event->angleDelta().y() / 120.0f;
    m_distance *= std::pow(0.85f, steps);
    m_distance = std::clamp(m_distance, m_radius * 0.05f, m_radius * 100.0f);
    updateProjection();
    update();
    event->accept();
}

void OcctViewWidget::uploadModel()
{
    if (!m_initialized || m_result.isNull()
            || m_result->vertices.size() != m_result->normals.size())
        return;

    QVector<float> data;
    data.reserve(m_result->vertices.size() * 6);
    for (qsizetype i = 0; i < m_result->vertices.size(); ++i)
    {
        const QVector3D &vertex = m_result->vertices.at(i);
        const QVector3D &normal = m_result->normals.at(i);
        data << vertex.x() << vertex.y() << vertex.z()
             << normal.x() << normal.y() << normal.z();
    }

    makeCurrent();
    m_vertexArray.bind();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(data.constData(), data.size() * int(sizeof(float)));
    m_program.bind();
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * int(sizeof(float)));
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3 * int(sizeof(float)), 3,
                                 6 * int(sizeof(float)));
    m_program.release();
    m_vertexBuffer.release();
    m_vertexArray.release();
    doneCurrent();

    m_vertexCount = m_result->vertices.size();
    update();
}

void OcctViewWidget::updateProjection()
{
    const float aspect = height() > 0 ? float(width()) / float(height()) : 1.0f;
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, aspect,
                             std::max(m_radius * 0.001f, 0.0001f),
                             std::max(m_radius * 200.0f, 10.0f));
}

void OcctViewWidget::setViewRotation(const QQuaternion &rotation)
{
    m_rotation = rotation;
    fitAll();
}
