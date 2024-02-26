#ifndef FILEMOVER_H
#define FILEMOVER_H

#include <QObject>
#include <QFileInfo>
#include <QFileInfoList>

#include "threadworker.h"

class ProgressDialog;
class FileMoverWorker;

class FileMover : public QObject
{
    Q_OBJECT
public:
    explicit FileMover(QWidget *parent = nullptr);
    void addSourceFile(const QFileInfo &fi);
    void addSourceFile(const QFileInfo &fi, const QString &dstSubdir);
    void addSourceFiles(QFileInfoList sourceFiles);
    void addSourceFiles(QFileInfoList sourceFiles, const QString &dstSubdir);
    void setDestination(const QString &dst);

public slots:
    void work();

private:
    QList<QPair<QFileInfo,QString>> m_sourceFiles;
    QString m_dst;
    ProgressDialog *m_progress;
    FileMoverWorker *m_mv;

private slots:
    void progressUpdate(int done, int total);
    void error(const QString &error);
    void confirmOverwriteFile(const QFileInfo &src, const QString &dst);
    void confirmReplaceDirectory(const QFileInfo &src, const QString &dst);
};

class FileMoverWorker : public ThreadWorker
{
    Q_OBJECT
public:
    enum Overwrite {
        ASK,
        NO,
        ONCE,
        YES_ALL,
        NO_ALL
    };

    explicit FileMoverWorker(QObject *parent = 0);
    void setSourceFiles(const QList<QPair<QFileInfo,QString>> &sourceFiles, const QString &dst);

public slots:
    void run();
    void continueWork(Overwrite overwrite = ASK);

signals:
    void fileExists(const QFileInfo &src, const QString &dst);
    void directoryExists(const QFileInfo &src, const QString &dst);

private:
    QList<QPair<QFileInfo,QString>> m_sourceFiles;
    QString m_dst;
    QList<QPair<QFileInfo, QString>> m_files;
    bool m_stopOnError;

    bool removeAll(const QString &path);
    bool removeRecursively(const QFileInfo &fi);
};

#endif // FILEMOVER_H
