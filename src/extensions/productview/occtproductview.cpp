#include "occtproductview.h"
#include "ui_occtproductview.h"

#include <QComboBox>
#include <QHideEvent>
#include <QPushButton>
#include <QThread>

OcctProductView::OcctProductView(QWidget *parent)
    : AbstractProductView(parent),
      ui(new Ui::OcctProductView),
      m_jobId(0)
{
    ui->setupUi(this);

    ui->modeComboBox->addItem(tr("Shaded"));
    ui->modeComboBox->addItem(tr("Wireframe"));
    ui->modeComboBox->setCurrentIndex(0);

    connect(ui->fitButton, &QPushButton::clicked,
            ui->viewer, &OcctViewWidget::fitAll);
    connect(ui->isometricButton, &QPushButton::clicked,
            ui->viewer, &OcctViewWidget::setIsometricView);
    connect(ui->frontButton, &QPushButton::clicked,
            ui->viewer, &OcctViewWidget::setFrontView);
    connect(ui->topButton, &QPushButton::clicked,
            ui->viewer, &OcctViewWidget::setTopView);
    connect(ui->rightButton, &QPushButton::clicked,
            ui->viewer, &OcctViewWidget::setRightView);
    connect(ui->modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (index == 0)
            ui->viewer->setShadedMode();
        else
            ui->viewer->setWireframeMode();
    });
    connect(ui->viewer, &OcctViewWidget::initializationFailed,
            this, [this](const QString &message) {
        ui->errorLabel->setText(message);
        ui->stackedWidget->setCurrentWidget(ui->errorPage);
        setControlsEnabled(false);
    });

    setControlsEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->loadingPage);
}

OcctProductView::~OcctProductView()
{
    cancelActiveWorker();
    delete ui;
}

QString OcctProductView::title()
{
    return tr("CAD part");
}

FileTypeList OcctProductView::canHandle()
{
    return FileTypeList() << FileType::STEP << FileType::IGES << FileType::STL;
}

bool OcctProductView::handle(FileMetadata *f)
{
    const QString absolutePath = f->fileInfo.absoluteFilePath();

    ++m_jobId;
    cancelActiveWorker();
    m_lastResult.clear();
    ui->viewer->clearScene();
    ui->errorLabel->clear();
    ui->loadingLabel->setText(tr("Loading CAD preview..."));
    ui->modeComboBox->setCurrentIndex(0);
    ui->stackedWidget->setCurrentWidget(ui->loadingPage);
    setControlsEnabled(false);

    startWorker(absolutePath, f->type);
    return true;
}

void OcctProductView::hideEvent(QHideEvent *event)
{
    ++m_jobId;
    cancelActiveWorker();
    AbstractProductView::hideEvent(event);
}

void OcctProductView::onImported(quint64 jobId, const OcctImportResultPtr &result)
{
    if (jobId != m_jobId)
        return;

    m_lastResult = result;
    ui->viewer->setImportedModel(result);
    ui->stackedWidget->setCurrentWidget(ui->viewerPage);
    setControlsEnabled(true);
}

void OcctProductView::onFailed(quint64 jobId, const QString &message)
{
    if (jobId != m_jobId)
        return;

    m_lastResult.clear();
    ui->viewer->clearScene();
    ui->errorLabel->setText(message);
    ui->stackedWidget->setCurrentWidget(ui->errorPage);
    setControlsEnabled(false);
}

void OcctProductView::onStatusChanged(quint64 jobId, const QString &message)
{
    if (jobId == m_jobId)
        ui->loadingLabel->setText(message);
}

void OcctProductView::cancelActiveWorker()
{
    QThread *thread = m_workerThread.data();
    OcctImportWorker *worker = m_worker.data();

    if (worker)
    {
        disconnect(worker, nullptr, this, nullptr);
        worker->stop();
    }

    if (thread && thread->isRunning())
    {
        thread->quit();
        thread->wait();
    }

    m_worker.clear();
    m_workerThread.clear();
}

void OcctProductView::setControlsEnabled(bool enabled)
{
    ui->fitButton->setEnabled(enabled);
    ui->isometricButton->setEnabled(enabled);
    ui->frontButton->setEnabled(enabled);
    ui->topButton->setEnabled(enabled);
    ui->rightButton->setEnabled(enabled);
    ui->modeComboBox->setEnabled(enabled);
}

void OcctProductView::startWorker(const QString &absolutePath, FileType::FileType fileType)
{
    QThread *thread = new QThread();
    OcctImportWorker *worker = new OcctImportWorker(absolutePath, fileType, m_jobId);
    worker->moveToThread(thread);

    connect(thread, &QThread::started,
            worker, &OcctImportWorker::run);
    connect(worker, &ThreadWorker::finished,
            thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, thread, worker]() {
        if (m_worker == worker)
            m_worker.clear();
        if (m_workerThread == thread)
            m_workerThread.clear();
    });
    connect(worker, &OcctImportWorker::imported,
            this, &OcctProductView::onImported,
            Qt::QueuedConnection);
    connect(worker, &OcctImportWorker::failed,
            this, &OcctProductView::onFailed,
            Qt::QueuedConnection);
    connect(worker, &OcctImportWorker::statusChanged,
            this, &OcctProductView::onStatusChanged,
            Qt::QueuedConnection);

    m_workerThread = thread;
    m_worker = worker;
    thread->start();
}
