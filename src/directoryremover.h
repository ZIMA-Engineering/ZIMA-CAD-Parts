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
	explicit DirectoryRemover(QWidget *parent = 0);
	explicit DirectoryRemover(QFileInfo fileInfo, QWidget *parent = 0);
	explicit DirectoryRemover(QFileInfoList fileInfos, QWidget *parent = 0);
	void addFiles(QFileInfoList fileInfos);
	void setMessage(const QString &msg);

public slots:
	void work();

private:
	QString m_msg;
	QFileInfoList m_fileInfos;
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
	void setFileInfos(const QFileInfoList &fileInfos);

public slots:
	void run();

private:
	QFileInfoList m_fileInfos;
	QFileInfoList m_files;

	void recurse(const QFileInfo &fi);
};

#endif // DIRECTORYREMOVER_H
