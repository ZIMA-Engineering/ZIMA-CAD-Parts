#ifndef FILECOPIER_H
#define FILECOPIER_H

#include <QObject>
#include <QFileInfo>
#include <QFileInfoList>

#include "threadworker.h"

class ProgressDialog;
class FileCopierWorker;

/*! Wraps the actual working thread that is deleting files and reporting
 * progress.
 */
class FileCopier : public QObject
{
    Q_OBJECT
public:
    explicit FileCopier(QWidget *parent = 0);
    explicit FileCopier(QFileInfo sourceFile, const QString &dst, QWidget *parent = 0);
    explicit FileCopier(QFileInfoList sourceFiles, const QString &dst, QWidget *parent = 0);
    void addSourceFile(const QFileInfo &fi);
    void addSourceFile(const QFileInfo &fi, const QString &dstSubdir);
    void addSourceFiles(QFileInfoList sourceFiles);
    void addSourceFiles(QFileInfoList sourceFiles, const QString &dstSubdir);
    void setDestination(const QString &dst);
    void setMessage(const QString &msg);
    void setStopOnError(bool stop);

public slots:
    void work();

private:
    QString m_msg;
    QList<QPair<QFileInfo,QString>> m_sourceFiles;
    QString m_dst;
    ProgressDialog *m_progress;
    FileCopierWorker *m_cp;
    bool m_stopOnError;

private slots:
    void progressUpdate(int done, int total);
    void directoryDeletionError(const QString &error);
    void confirmOverwrite(const QFileInfo &src, const QString &dst);
};

class FileCopierWorker : public ThreadWorker
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

    explicit FileCopierWorker(QObject *parent = 0);
    void setSourceFiles(const QList<QPair<QFileInfo,QString>> &sourceFiles, const QString &dst);
    void setStopOnError(bool stop);

public slots:
    void run();
    void continueWork(Overwrite overwrite = ASK);

signals:
    void fileExists(const QFileInfo &src, const QString &dst);

private:
    QList<QPair<QFileInfo,QString>> m_sourceFiles;
    QString m_dst;
    QList<QPair<QFileInfo, QString>> m_files;
    bool m_stopOnError;

    void recurse(const QFileInfo &src, const QString &dst, const QString &subdir);
};

#endif // FILECOPIER_H
