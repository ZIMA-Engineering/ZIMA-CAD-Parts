#include "directorycreator.h"
#include "settings.h"
#include "progressdialog.h"

#include <QDir>
#include <QLabel>
#include <QProgressBar>
#include <QDebug>

DirectoryCreator::DirectoryCreator(const QString &dir, const QString &name, QObject *parent) :
	QObject(parent),
	m_dir(dir),
	m_name(name)
{

}

void DirectoryCreator::setPrototype(const QString &prototype)
{
	m_prototype = prototype;
}

void DirectoryCreator::work()
{
	auto worker = ThreadWorker::create<DirectoryCreatorThread>();
	worker->setInfo(m_dir, m_name, m_prototype);

	auto progress = new ProgressDialog(static_cast<QWidget*>(parent()));
	progress->label()->setText(tr("Please wait while the directory is being created..."));
	progress->progressBar()->setMinimum(0);
	progress->progressBar()->setMaximum(0);

	// Quit after worker finishes
	connect(worker, SIGNAL(finished()),
			progress, SLOT(accept()));

	worker->start();

	if (progress->exec() == QDialog::Rejected)
		worker->stop();

	worker->deleteLater();
	progress->deleteLater();
}

void DirectoryCreatorThread::setInfo(const QString &dir, const QString &name, const QString &prototype)
{
	m_dir = dir;
	m_name = name;
	m_prototype = prototype;
}

void DirectoryCreatorThread::run()
{
	QDir d(m_dir);
	d.mkdir(m_name);

	if (m_prototype.isEmpty())
	{
		d.mkdir(m_name + "/" + METADATA_DIR);
		emit finished();
		return;
	}

	// Copy prototype contents
	recursiveCopy(m_dir + "/" PROTOTYPE_DIR + "/" + m_prototype, m_dir + "/" + m_name);

	// Ensure 0000-index is created, in case it's not in the prototype
	d.mkpath(m_name + "/" + METADATA_DIR);

	emit finished();
}

void DirectoryCreatorThread::recursiveCopy(const QString &src, const QString &dst)
{
	QDir d_src(src);
	QDir d_dst(dst);
	QFileInfoList list = d_src.entryInfoList(
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
		{
			qDebug() << "Mkdir" << (d_dst.absolutePath() + "/" + f.baseName());
			d_dst.mkdir(f.baseName());
			recursiveCopy(f.absoluteFilePath(), dst + "/" + f.baseName());
		}

		qDebug() << "Copy" << f.absoluteFilePath() << "to" << (dst + "/" + f.fileName());
		QFile::copy(f.absoluteFilePath(), dst + "/" + f.fileName());
	}
}
