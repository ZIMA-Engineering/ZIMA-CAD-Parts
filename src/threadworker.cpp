#include "threadworker.h"

ThreadWorker::ThreadWorker(QObject *parent) : QObject(parent)
{

}

void ThreadWorker::setup()
{
    // Start/quit thread
    connect(thread(), SIGNAL(started()),
            this, SLOT(run()));
    connect(this, SIGNAL(finished()),
            thread(), SLOT(quit()));

    // Quit after thread finishes
    connect(thread(), SIGNAL(finished()),
            this, SIGNAL(finished()));
}

void ThreadWorker::start()
{
    thread()->start();
}

void ThreadWorker::stop()
{
    QThread::currentThread()->requestInterruption();
}

void ThreadWorker::quit()
{
    thread()->quit();
}

bool ThreadWorker::shouldStop() const
{
    return QThread::currentThread()->isInterruptionRequested();
}
