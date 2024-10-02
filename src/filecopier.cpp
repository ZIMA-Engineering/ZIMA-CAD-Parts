#include "filecopier.h"
#include "progressdialog.h"

#include <QDir>
#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QMessageBox>

FileCopier::FileCopier(QWidget *parent)
    : FileCopier(QFileInfoList(), QString(), parent)
{

}

FileCopier::FileCopier(QFileInfo sourceFile, const QString &dst, QWidget *parent)
    : FileCopier(QFileInfoList() << sourceFile, dst, parent)
{

}

FileCopier::FileCopier(QFileInfoList sourceFiles, const QString &dst, QWidget *parent)
    : QObject(parent),
      m_dst(dst),
      m_stopOnError(true)
{
    foreach (const QFileInfo &fi, sourceFiles)
        m_sourceFiles << FileCopyIntent(fi);
}

void FileCopier::addSourceFile(const QFileInfo &fi)
{
    m_sourceFiles << FileCopyIntent(fi);
}

void FileCopier::addSourceFile(const QFileInfo &fi, const QString &dstSubdir)
{
    m_sourceFiles << FileCopyIntent(fi, dstSubdir);
}

void FileCopier::addSourceFiles(QFileInfoList sourceFiles)
{
    foreach (const QFileInfo &fi, sourceFiles)
        addSourceFile(fi);
}

void FileCopier::addSourceFiles(QFileInfoList sourceFiles, const QString &dstSubdir)
{
    foreach (const QFileInfo &fi, sourceFiles)
        addSourceFile(fi, dstSubdir);
}

void FileCopier::addSourceDirectoryContents(const QFileInfo &fi)
{
    m_sourceFiles << FileCopyIntent(fi, QString(), true);
}

void FileCopier::setDestination(const QString &dst)
{
    m_dst = dst;
}

void FileCopier::setMessage(const QString &msg)
{
    m_msg = msg;
}

void FileCopier::setStopOnError(bool stop)
{
    m_stopOnError = stop;
}

void FileCopier::work()
{
    m_cp = ThreadWorker::create<FileCopierWorker>();
    m_cp->setSourceFiles(m_sourceFiles, m_dst);
    m_cp->setStopOnError(m_stopOnError);

    m_progress = new ProgressDialog(static_cast<QWidget*>(parent()));
    m_progress->label()->setText(
        m_msg.isNull() ? tr("Please wait while the files are being copied...") : m_msg
    );

    // Quit after worker finishes
    connect(m_cp, SIGNAL(finished()),
            m_progress, SLOT(accept()));

    // Monitor progress
    connect(m_cp, SIGNAL(progress(int,int)),
            this, SLOT(progressUpdate(int,int)));
    connect(m_cp, SIGNAL(errorOccured(QString)),
            this, SLOT(directoryDeletionError(QString)));
    connect(m_cp, SIGNAL(fileExists(QFileInfo,QString)),
            this, SLOT(confirmOverwrite(QFileInfo,QString)));

    m_cp->start();

    if (m_progress->exec() == QDialog::Rejected)
        m_cp->stop();

    m_cp->deleteLater();
    m_progress->deleteLater();
}

void FileCopier::progressUpdate(int done, int total)
{
    Q_UNUSED(total)

    m_progress->progressBar()->setValue(done);
}

void FileCopier::directoryDeletionError(const QString &error)
{
    QMessageBox::warning(
        static_cast<QWidget*>(parent()),
        tr("Directory deletion failed"),
        error
    );
}

void FileCopier::confirmOverwrite(const QFileInfo &src, const QString &dst)
{
    Q_UNUSED(src)

    QMessageBox::StandardButton ret = QMessageBox::question(
                                          0,
                                          tr("Overwrite file?"),
                                          tr("File %1 already exists. Overwrite?").arg(dst),
                                          QMessageBox::Yes
                                          | QMessageBox::No
                                          | QMessageBox::NoAll
                                          | QMessageBox::YesAll
                                          | QMessageBox::Cancel
                                      );

    switch (ret)
    {
    case QMessageBox::Yes:
        m_cp->continueWork(FileCopierWorker::ONCE);
        break;
    case QMessageBox::No:
        m_cp->continueWork(FileCopierWorker::NO);
        break;
    case QMessageBox::NoAll:
        m_cp->continueWork(FileCopierWorker::NO_ALL);
        break;
    case QMessageBox::YesAll:
        m_cp->continueWork(FileCopierWorker::YES_ALL);
        break;
    case QMessageBox::Cancel:
        m_progress->reject();
        break;
    default:
        break;
    }
}


FileCopierWorker::FileCopierWorker(QObject *parent)
    : ThreadWorker(parent)
{

}

void FileCopierWorker::setSourceFiles(const QList<FileCopyIntent> &sourceFiles, const QString &dst)
{
    m_sourceFiles = sourceFiles;
    m_dst = dst;
}

void FileCopierWorker::setStopOnError(bool stop)
{
    m_stopOnError = stop;
}

void FileCopierWorker::run()
{
    foreach (const FileCopyIntent &intent, m_sourceFiles)
        recurse(intent.sourceFile, m_dst, intent.dstSubdir, intent.contents);

    continueWork();
}

void FileCopierWorker::continueWork(FileCopierWorker::Overwrite overwrite)
{
    int i = 0, done = 0, lastDone = 0;

    qDebug() << "Going to copy" << m_files.count() << "files";

    while (!m_files.isEmpty())
    {
        QPair<QFileInfo, QString> pair = m_files.first();
        QFileInfo src = pair.first;
        QString dst = pair.second;
        QDir d;
        QFile file;
        bool ret;

        if (shouldStop())
        {
            quit();
            return;
        }

        if (src.isDir() && !src.isSymLink())
        {
            qDebug() << "Create dir" << dst;

            ret = d.mkpath(dst);

        } else {
            if (QFile::exists(dst))
            {
                switch (overwrite)
                {
                case FileCopierWorker::ASK:
                    emit fileExists(src, dst);
                    return;

                case FileCopierWorker::NO:
                    overwrite = FileCopierWorker::ASK;
                    m_files.removeFirst();
                    continue;

                case FileCopierWorker::ONCE:
                    QFile::remove(dst);
                    overwrite = FileCopierWorker::ASK;
                    break;

                case FileCopierWorker::YES_ALL:
                    QFile::remove(dst);
                    break;

                case FileCopierWorker::NO_ALL:
                    m_files.removeFirst();
                    continue;
                }
            }

            qDebug() << "Copy" << src.absoluteFilePath() << "into" << dst;

            d.mkpath(QFileInfo(dst).absoluteDir().absolutePath());
            file.setFileName(src.absoluteFilePath());
            ret = file.copy(dst);

            if (!ret)
                qDebug() << file.error() << file.errorString();
        }

        if (!ret)
        {
            emit errorOccured(
                tr("Unable to copy '%1': %2")
                .arg(src.absoluteFilePath())
                .arg(file.errorString())
            );

            if (m_stopOnError)
            {
                quit();
                return;
            }
        }

        done = ++i / (double) m_files.size() * 100;

        if (done > lastDone)
        {
            lastDone = done;
            emit progress(done, 100);
        }

        m_files.removeFirst();
    }

    emit finished();
}

void FileCopierWorker::recurse(const QFileInfo &src, const QString &dst, const QString &subdir, bool contents)
{
    if (contents)
    {
        m_files << QPair<QFileInfo, QString>(
                    src,
                    dst + "/" + subdir
                );
    } else {
        m_files << QPair<QFileInfo, QString>(
                    src,
                    dst + "/" + subdir + "/" + src.fileName()
                );
    }

    if (!src.isDir())
        return;

    QDir root(src.absoluteFilePath());
    QFileInfoList list = root.entryInfoList(
                             QDir::NoDotAndDotDot
                             | QDir::AllEntries
                             | QDir::Hidden
                             | QDir::System
                         );

    foreach (const QFileInfo &f, list)
    {
        if (shouldStop())
            return;

        QString dstPath;

        if (contents) {
            dstPath = dst + "/" + subdir;
        } else {
            dstPath = dst + "/" + subdir + "/" + src.fileName();
        }

        if (f.isDir())
        {
            recurse(f, dstPath, QString(), false);
        } else {
            m_files << QPair<QFileInfo, QString>(
                        f,
                        dstPath + "/" + f.fileName()
                    );
        }
    }
}
