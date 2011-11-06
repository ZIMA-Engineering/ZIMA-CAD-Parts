#ifndef LOCALDATASOURCE_H
#define LOCALDATASOURCE_H

#include <QThread>
#include "basedatasource.h"

class LocalCopier : public QThread
{
Q_OBJECT
public:
	LocalCopier(QList<File*> files, QString dir);
	LocalCopier(File* file, QString dir);
	void run();
	void addFiles(QList<File*> files);

private:
	QList<File*> files;
	QString target;

signals:
	void aboutToCopy(File *file);
	void fileCopied(File *file);
	void done();
	void progress(File*);
};

class LocalDataSource : public BaseDataSource
{
	Q_OBJECT
public:
	explicit LocalDataSource(QObject *parent = 0);
	QString internalName();
	QString localPath;

public slots:
	void loadDirectory(Item* item);
	void sendTechSpecUrl(Item* item);
	void downloadFiles(QList<File*> files, QString dir);
	void downloadFile(File* file);
	void resumeDownload();
	void abort();
	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings);

protected:
	LocalCopier *copier;
protected slots:
	void aboutToCopy(File *file);
};

#endif // LOCALDATASOURCE_H
