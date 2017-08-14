#ifndef DIRECTORYREMOVER_H
#define DIRECTORYREMOVER_H

#include <QObject>
#include <QFileInfo>
#include <QFileInfoList>

#include "threadworker.h"

class ProgressDialog;
class DirectoryRemoverWorker;

/*! Wraps the actual working thread that is deleting files and reporting
 * progress.
 */
class DirectoryRemover : public QObject
{
	Q_OBJECT
public:
	explicit DirectoryRemover(QFileInfo dirInfo, QWidget *parent = 0);

public slots:
	void work();

private:
	QFileInfo m_dirInfo;
	ProgressDialog *m_progress;
	DirectoryRemoverWorker *m_rm;

private slots:
	void progressUpdate(int done, int total);
	void directoryDeletionError(const QString &error);
};

class DirectoryRemoverWorker : public ThreadWorker
{
	Q_OBJECT
public:
	explicit DirectoryRemoverWorker(QObject *parent = 0);
	void setDirInfo(const QFileInfo &dirInfo);

public slots:
	void run();

private:
	QFileInfo m_dirInfo;
	QFileInfoList m_files;

	void recurse(const QFileInfo &dir);
};

#endif // DIRECTORYREMOVER_H
