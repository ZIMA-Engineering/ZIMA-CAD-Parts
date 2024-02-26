#ifndef PRTREADER_H
#define PRTREADER_H

#include <QObject>
#include <QThread>
#include <QFileInfoList>

class PtrReaderThread : public QThread
{
    Q_OBJECT

public:
    PtrReaderThread(QFileInfoList partList);

signals:
    void partParam(const QString &part, const QString &param, const QString &value);

protected:
    void run();

private:
    QFileInfoList m_partList;

    void parseFile(const QFileInfo &fi);
};

class PrtReader : public QObject
{
    Q_OBJECT
public:
    explicit PrtReader(QObject *parent = nullptr);
    bool isRunning() const;

signals:
    void loaded(const QFileInfo &part);

public slots:
    void load(const QString &dir, const QFileInfoList &partList);
    void stop();

private:
    PtrReaderThread *m_thread;
    QString m_dir;

private slots:
    void setPartParam(const QString &part, const QString &param, const QString &value);
};

#endif // PRTREADER_H
