#ifndef DIRECTORYREMOVER_H
#define DIRECTORYREMOVER_H

#include <QObject>
#include <QFileInfo>
#include <QFileInfoList>
#include <QMutexLocker>

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

private slots:
	void directoryDeletionError(const QFileInfo &file);
};

class DirectoryRemoverWorker : public QObject
{
	Q_OBJECT
public:
	explicit DirectoryRemoverWorker(QFileInfo dirInfo, QObject *parent = 0);

signals:
	void progress(int done);
	void errorOccured(const QFileInfo &file);
	void finished();

public slots:
	void start();
	void stop();

private:
	QMutex m_mutex;
	QFileInfo m_dirInfo;
	QFileInfoList m_files;
	bool m_stop;

	void recurse(const QFileInfo &dir);
	bool shouldStop();
};

#endif // DIRECTORYREMOVER_H
