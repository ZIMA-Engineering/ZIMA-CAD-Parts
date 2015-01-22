#include "directoryremover.h"
#include "progressdialog.h"

#include <QDir>
#include <QDebug>
#include <QThread>
#include <QProgressBar>
#include <QLabel>
#include <QMessageBox>

DirectoryRemover::DirectoryRemover(QFileInfo dirInfo, QWidget *parent)
	: QObject(parent),
	  m_dirInfo(dirInfo)
{

}

void DirectoryRemover::work()
{
	QThread *thread = new QThread(this);
	DirectoryRemoverWorker *rm = new DirectoryRemoverWorker(m_dirInfo);
	ProgressDialog *progress = new ProgressDialog(static_cast<QWidget*>(parent()));

	progress->label()->setText(tr("Please wait while the directory is being removed..."));

	rm->moveToThread(thread);

	// Start/quit thread
	connect(thread, SIGNAL(started()),
			rm, SLOT(start()));
	connect(rm, SIGNAL(finished()),
			thread, SLOT(quit()));

	// Quit after thread finishes
	connect(thread, SIGNAL(finished()),
			rm, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()),
			progress, SLOT(accept()));
	connect(thread, SIGNAL(finished()),
			progress, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()),
			this, SLOT(deleteLater()));

	// Monitor progress
	connect(rm, SIGNAL(progress(int)),
			progress->progressBar(), SLOT(setValue(int)));
	connect(rm, SIGNAL(errorOccured(QFileInfo)),
			this, SLOT(directoryDeletionError(QFileInfo)));
	connect(rm, SIGNAL(errorOccured(QFileInfo)),
			thread, SLOT(quit()));

	thread->start();

	if (progress->exec() == QDialog::Rejected)
		rm->stop();
}

void DirectoryRemover::directoryDeletionError(const QFileInfo &file)
{
	QMessageBox::warning(static_cast<QWidget*>(parent()), tr("Directory deletion failed"),
						 tr("Unable to delete '%1'").arg(file.absoluteFilePath()));
}


DirectoryRemoverWorker::DirectoryRemoverWorker(QFileInfo dirInfo, QObject *parent)
	: QObject(parent),
	  m_dirInfo(dirInfo),
	  m_stop(false)
{

}

void DirectoryRemoverWorker::start()
{
	recurse(m_dirInfo);

	int i = 0, done = 0, lastDone = 0;

	qDebug() << "Going to delete" << m_files.count() << "files";

	foreach (const QFileInfo &f, m_files)
	{
		bool ret;

		if (shouldStop())
			return;

		qDebug() << "Remove" << f.absoluteFilePath();

		if (f.isDir() && !f.isSymLink())
		{
			ret = f.absoluteDir().rmdir(f.fileName());

		} else {
			ret = QFile::remove(f.absoluteFilePath());
		}

		if (!ret)
		{
			emit errorOccured(f);
			return;
		}

		done = ++i / (double) m_files.size() * 100;

		if (done > lastDone)
		{
			lastDone = done;
			emit progress(done);
		}
	}

	emit finished();
}

void DirectoryRemoverWorker::stop()
{
	QMutexLocker locker(&m_mutex);

	m_stop = true;
}

void DirectoryRemoverWorker::recurse(const QFileInfo &dir)
{
	QDir root(dir.absoluteFilePath());
	QFileInfoList list = root.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);

	foreach (const QFileInfo &f, list)
	{
		if (shouldStop())
			return;

		if (f.isDir())
			recurse(f);

		else
			m_files << f;
	}

	m_files << dir;
}

bool DirectoryRemoverWorker::shouldStop()
{
	QMutexLocker locker(&m_mutex);

	return m_stop;
}
