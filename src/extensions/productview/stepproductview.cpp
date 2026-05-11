#include "stepproductview.h"
#include "ui_stepproductview.h"

#include <QApplication>
#include <QComboBox>
#include <QHideEvent>
#include <QPushButton>
#include <QThread>

StepProductView::StepProductView(QWidget *parent)
    : AbstractProductView(parent),
      ui(new Ui::StepProductView),
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

    setControlsEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->loadingPage);
}

StepProductView::~StepProductView()
{
    cancelActiveWorker();
    delete ui;
}

QString StepProductView::title()
{
    return tr("STEP part");
}

FileTypeList StepProductView::canHandle()
{
    return FileTypeList() << FileType::STEP;
}

bool StepProductView::handle(FileMetadata *f)
{
    const QString absolutePath = f->fileInfo.absoluteFilePath();

    ++m_jobId;
    cancelActiveWorker();
    m_lastResult.clear();
    ui->viewer->clearScene();
    ui->errorLabel->clear();
    ui->loadingLabel->setText(tr("Loading STEP preview..."));
    ui->modeComboBox->setCurrentIndex(0);
    ui->stackedWidget->setCurrentWidget(ui->loadingPage);
    setControlsEnabled(false);

    startWorker(absolutePath);
    return true;
}

void StepProductView::hideEvent(QHideEvent *event)
{
    ++m_jobId;
    cancelActiveWorker();
    AbstractProductView::hideEvent(event);
}

void StepProductView::onImported(quint64 jobId, const StepImportResultPtr &result)
{
    if (jobId != m_jobId)
        return;

    m_lastResult = result;
    ui->viewer->setImportedDocument(result);
    ui->stackedWidget->setCurrentWidget(ui->viewerPage);
    setControlsEnabled(true);
}

void StepProductView::onFailed(quint64 jobId, const QString &message)
{
    if (jobId != m_jobId)
        return;

    m_lastResult.clear();
    ui->viewer->clearScene();
    ui->errorLabel->setText(message);
    ui->stackedWidget->setCurrentWidget(ui->errorPage);
    setControlsEnabled(false);
}

void StepProductView::cancelActiveWorker()
{
    if (m_worker)
    {
        disconnect(m_worker, nullptr, this, nullptr);
        m_worker->stop();
        m_worker.clear();
    }

    m_workerThread.clear();
}

void StepProductView::setControlsEnabled(bool enabled)
{
    ui->fitButton->setEnabled(enabled);
    ui->isometricButton->setEnabled(enabled);
    ui->frontButton->setEnabled(enabled);
    ui->topButton->setEnabled(enabled);
    ui->rightButton->setEnabled(enabled);
    ui->modeComboBox->setEnabled(enabled);
}

void StepProductView::startWorker(const QString &absolutePath)
{
    QThread *thread = new QThread(qApp);
    StepImportWorker *worker = new StepImportWorker(absolutePath, m_jobId);
    worker->moveToThread(thread);
    worker->setup();

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(worker, &StepImportWorker::imported,
            this, &StepProductView::onImported,
            Qt::QueuedConnection);
    connect(worker, &StepImportWorker::failed,
            this, &StepProductView::onFailed,
            Qt::QueuedConnection);

    m_workerThread = thread;
    m_worker = worker;
    thread->start();
}
