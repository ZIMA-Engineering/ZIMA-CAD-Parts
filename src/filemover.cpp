#include "filemover.h"
#include "progressdialog.h"

#include <QDir>
#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QMessageBox>

FileMover::FileMover(QWidget *parent)
	: QObject(parent)
{
}

void FileMover::addSourceFile(const QFileInfo &fi)
{
	m_sourceFiles << QPair<QFileInfo, QString>(fi, QString());
}

void FileMover::addSourceFile(const QFileInfo &fi, const QString &dstSubdir)
{
	m_sourceFiles << QPair<QFileInfo, QString>(fi, dstSubdir);
}

void FileMover::setDestination(const QString &dst)
{
	m_dst = dst;
}

void FileMover::work()
{
	m_mv = ThreadWorker::create<FileMoverWorker>();
	m_mv->setSourceFiles(m_sourceFiles, m_dst);

	m_progress = new ProgressDialog(static_cast<QWidget*>(parent()));
	m_progress->label()->setText(
		tr("Please wait while the files are being moved...")
	);

	// Quit after worker finishes
	connect(m_mv, SIGNAL(finished()),
			m_progress, SLOT(accept()));

	// Monitor progress
	connect(m_mv, SIGNAL(progress(int,int)),
			this, SLOT(progressUpdate(int,int)));
	connect(m_mv, SIGNAL(errorOccured(QString)),
			this, SLOT(error(QString)));
	connect(m_mv, SIGNAL(fileExists(QFileInfo,QString)),
			this, SLOT(confirmOverwriteFile(QFileInfo,QString)));
	connect(m_mv, SIGNAL(directoryExists(QFileInfo,QString)),
			this, SLOT(confirmReplaceDirectory(QFileInfo,QString)));

	m_mv->start();

	if (m_progress->exec() == QDialog::Rejected)
		m_mv->stop();

	m_mv->deleteLater();
	m_progress->deleteLater();
}

void FileMover::progressUpdate(int done, int total)
{
	Q_UNUSED(total)

	m_progress->progressBar()->setValue(done);
}

void FileMover::error(const QString &error)
{
	QMessageBox::warning(
		static_cast<QWidget*>(parent()),
		tr("File move failed"),
		error
	);
}

void FileMover::confirmOverwriteFile(const QFileInfo &src, const QString &dst)
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
		m_mv->continueWork(FileMoverWorker::ONCE);
		break;
	case QMessageBox::No:
		m_mv->continueWork(FileMoverWorker::NO);
		break;
	case QMessageBox::NoAll:
		m_mv->continueWork(FileMoverWorker::NO_ALL);
		break;
	case QMessageBox::YesAll:
		m_mv->continueWork(FileMoverWorker::YES_ALL);
		break;
	case QMessageBox::Cancel:
		m_progress->reject();
		break;
	default:
		break;
	}
}

void FileMover::confirmReplaceDirectory(const QFileInfo &src, const QString &dst)
{
	Q_UNUSED(src)

	QMessageBox::StandardButton ret = QMessageBox::question(
		0,
		tr("Replace directory?"),
		tr("Directory %1 already exists. Replace it?").arg(dst),
		QMessageBox::Yes
			| QMessageBox::No
			| QMessageBox::NoAll
			| QMessageBox::YesAll
			| QMessageBox::Cancel
	);

	switch (ret)
	{
	case QMessageBox::Yes:
		m_mv->continueWork(FileMoverWorker::ONCE);
		break;
	case QMessageBox::No:
		m_mv->continueWork(FileMoverWorker::NO);
		break;
	case QMessageBox::NoAll:
		m_mv->continueWork(FileMoverWorker::NO_ALL);
		break;
	case QMessageBox::YesAll:
		m_mv->continueWork(FileMoverWorker::YES_ALL);
		break;
	case QMessageBox::Cancel:
		m_progress->reject();
		break;
	default:
		break;
	}
}

FileMoverWorker::FileMoverWorker(QObject *parent)
	: ThreadWorker(parent)
{

}

void FileMoverWorker::setSourceFiles(const QList<QPair<QFileInfo, QString> > &sourceFiles, const QString &dst)
{
	m_sourceFiles = sourceFiles;
	m_dst = dst;
}

void FileMoverWorker::run()
{
	QPair<QFileInfo, QString> pair;

	foreach (pair, m_sourceFiles)
	{
		const QFileInfo &src = pair.first;
		const QString &subdir = pair.second;

		m_files << QPair<QFileInfo, QString>(
			src,
			m_dst + "/" + subdir + "/" + src.fileName()
		);
	}

	continueWork();
}

void FileMoverWorker::continueWork(FileMoverWorker::Overwrite overwrite)
{
	int i = 0, done = 0, lastDone = 0;

	qDebug() << "Going to move" << m_files.count() << "files";

	while (!m_files.isEmpty())
	{
		QPair<QFileInfo, QString> pair = m_files.first();
		QFileInfo src = pair.first;
		QString dst = pair.second;
		QFileInfo fi(dst);
		QDir d;
		QFile file;
		bool ret;

		if (shouldStop())
		{
			quit();
			return;
		}

		if (QFile::exists(dst))
		{
			switch (overwrite)
			{
			case FileMoverWorker::ASK:
				if (fi.isDir())
					emit directoryExists(src, dst);
				else
					emit fileExists(src, dst);
				return;

			case FileMoverWorker::NO:
				overwrite = FileMoverWorker::ASK;
				m_files.removeFirst();
				continue;

			case FileMoverWorker::ONCE:
				if (!removeAll(dst))
					return;
				overwrite = FileMoverWorker::ASK;
				break;

			case FileMoverWorker::YES_ALL:
				if (!removeAll(dst))
					return;
				break;

			case FileMoverWorker::NO_ALL:
				m_files.removeFirst();
				continue;
			}
		}

		qDebug() << "Move" << src.absoluteFilePath() << "into" << dst;

		file.setFileName(src.absoluteFilePath());
		ret = file.rename(dst);

		if (!ret)
			qDebug() << file.error() << file.errorString();

		if (!ret)
		{
			emit errorOccured(
				tr("Unable to move '%1': %2")
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

bool FileMoverWorker::removeAll(const QString &path)
{
	QFileInfo fi(path);

	qDebug() << "Removing directory" << path;

	if (fi.isDir())
	{
		if (!removeRecursively(fi))
		{
			emit errorOccured(
				tr("Unable to remove '%1'")
				.arg(fi.absoluteFilePath())
			);
			return false;
		}

		qDebug() << "rmdir" << fi.dir().absolutePath();

		if (!fi.dir().rmdir(fi.fileName()))
		{
			emit errorOccured(
				tr("Unable to remove '%1'")
				.arg(fi.absoluteFilePath())
			);
			return false;
		}

		return true;

	} else {
		qDebug() << "rm" << path;

		if (QFile::remove(path)) {
			return true;
		} else {
			emit errorOccured(
				tr("Unable to remove '%1'")
				.arg(fi.absoluteFilePath())
			);
			return false;
		}
	}
}

bool FileMoverWorker::removeRecursively(const QFileInfo &fi)
{
	QDir d(fi.absoluteFilePath());
	QFileInfoList entries = d.entryInfoList(
		QDir::NoDotAndDotDot
		| QDir::AllEntries
		| QDir::Hidden
		| QDir::System
	);

	foreach (const QFileInfo &fi, entries)
	{
		if (fi.isDir())
		{
			if (!removeRecursively(fi))
				return false;

			qDebug() << "rmdir" << fi.dir().absolutePath();

			if (!fi.dir().rmdir(fi.fileName()))
			{
				qDebug() << "failed to remove" << fi.absoluteFilePath();
				return false;
			}

		} else {
			qDebug() << "rm" << fi.absoluteFilePath();

			if (!QFile::remove(fi.absoluteFilePath()))
			{
				qDebug() << "failed to remove" << fi.absoluteFilePath();
				return false;
			}
		}
	}

	return true;
}
