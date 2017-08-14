#include "directoryremover.h"
#include "progressdialog.h"

#include <QDir>
#include <QDebug>
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
	m_rm = ThreadWorker::create<DirectoryRemoverWorker>();
	m_rm->setDirInfo(m_dirInfo);

	m_progress = new ProgressDialog(static_cast<QWidget*>(parent()));
	m_progress->label()->setText(tr("Please wait while the directory is being removed..."));

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

void DirectoryRemoverWorker::setDirInfo(const QFileInfo &dirInfo)
{
	m_dirInfo = dirInfo;
}

void DirectoryRemoverWorker::run()
{
	recurse(m_dirInfo);

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
			quit();
			return;
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
