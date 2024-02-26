#include "directoryremover.h"
#include "progressdialog.h"

#include <QDir>
#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QMessageBox>

DirectoryRemover::DirectoryRemover(QWidget *parent)
    : DirectoryRemover(QFileInfoList(), parent)
{

}

DirectoryRemover::DirectoryRemover(QFileInfo fileInfo, QWidget *parent)
    : DirectoryRemover(QFileInfoList() << fileInfo, parent)
{

}

DirectoryRemover::DirectoryRemover(QFileInfoList fileInfos, QWidget *parent)
    : QObject(parent),
      m_fileInfos(fileInfos),
      m_stopOnError(true)
{

}

void DirectoryRemover::addFiles(QFileInfoList fileInfos)
{
    m_fileInfos << fileInfos;
}

void DirectoryRemover::setMessage(const QString &msg)
{
    m_msg = msg;
}

void DirectoryRemover::setStopOnError(bool stop)
{
    m_stopOnError = stop;
}

void DirectoryRemover::work()
{
    m_rm = ThreadWorker::create<DirectoryRemoverWorker>();
    m_rm->setFileInfos(m_fileInfos);
    m_rm->setStopOnError(m_stopOnError);

    m_progress = new ProgressDialog(static_cast<QWidget*>(parent()));
    m_progress->label()->setText(
        m_msg.isNull() ? tr("Please wait while the files are being removed...") : m_msg
    );

    // Quit after worker finishes
    connect(m_rm, SIGNAL(finished()),
            m_progress, SLOT(accept()));

    // Monitor progress
    connect(m_rm, SIGNAL(progress(int,int)),
            this, SLOT(progressUpdate(int,int)));
    connect(m_rm, SIGNAL(errorOccured(QString)),
            this, SLOT(directoryDeletionError(QString)));

    m_rm->start();

    if (m_progress->exec() == QDialog::Rejected)
        m_rm->stop();

    m_rm->deleteLater();
    m_progress->deleteLater();
}

void DirectoryRemover::progressUpdate(int done, int total)
{
    Q_UNUSED(total)

    m_progress->progressBar()->setValue(done);
}

void DirectoryRemover::directoryDeletionError(const QString &error)
{
    QMessageBox::warning(
        static_cast<QWidget*>(parent()),
        tr("Directory deletion failed"),
        error
    );
}


DirectoryRemoverWorker::DirectoryRemoverWorker(QObject *parent)
    : ThreadWorker(parent)
{

}

void DirectoryRemoverWorker::setFileInfos(const QFileInfoList &fileInfos)
{
    m_fileInfos = fileInfos;
}

void DirectoryRemoverWorker::setStopOnError(bool stop)
{
    m_stopOnError = stop;
}

void DirectoryRemoverWorker::run()
{
    foreach (const QFileInfo &fi, m_fileInfos)
        recurse(fi);

    int i = 0, done = 0, lastDone = 0;

    qDebug() << "Going to delete" << m_files.count() << "files";

    foreach (const QFileInfo &f, m_files)
    {
        bool ret;

        if (shouldStop())
        {
            quit();
            return;
        }

        qDebug() << "Remove" << f.absoluteFilePath();

        if (f.isDir() && !f.isSymLink())
        {
            ret = f.absoluteDir().rmdir(f.fileName());

        } else {
            ret = QFile::remove(f.absoluteFilePath());
        }

        if (!ret)
        {
            emit errorOccured(tr("Unable to delete '%1'").arg(f.absoluteFilePath()));

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
    }

    emit finished();
}

void DirectoryRemoverWorker::recurse(const QFileInfo &fi)
{
    if (!fi.isDir())
    {
        m_files << fi;
        return;
    }

    QDir root(fi.absoluteFilePath());
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

        if (f.isDir())
            recurse(f);

        else
            m_files << f;
    }

    m_files << fi;
}
