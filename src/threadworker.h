#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <QObject>
#include <QThread>

/*!
 * \brief Generic worker for background tasks
 *
 * This class can be used whenever you need to do some computation
 * or I/O heavy work and not block the UI thread. All you need to
 * implement is the \c run() method. Worker instances are created
 * using \c create(), which will move the worker into a new thread.
 */
class ThreadWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThreadWorker(QObject *parent = nullptr);
    template <class T>
    static T* create();
    void setup();

signals:
    void progress(int done, int total);
    void errorOccured(const QString &error);
    void finished();

public slots:
    void start();
    virtual void run() = 0;
    virtual void stop();
    void quit();

protected:
    bool shouldStop() const;
};

template<class T>
T* ThreadWorker::create()
{
    T *worker = new T();
    QThread *thread = new QThread();
    worker->moveToThread(thread);
    worker->setup();

    return worker;
}

#endif // THREADWORKER_H
