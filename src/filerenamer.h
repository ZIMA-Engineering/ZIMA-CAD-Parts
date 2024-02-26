#ifndef FILERENAMER_H
#define FILERENAMER_H

#include <QObject>
#include <QFileInfo>

class FileRenamer : public QObject
{
    Q_OBJECT

public:
    FileRenamer(QObject *parent = nullptr);
    bool rename(const QString &dir, const QFileInfo &file, QString newName);

signals:
    void renamed(const QFileInfo &oldFile, const QFileInfo &newFile);
    void error(const QFileInfo &oldFile, const QFileInfo &newFile, const QString &error);

private:
    bool renameFilesInDir(const QString &dir, const QString &oldName, const QString &newName);
};

#endif // FILERENAMER_H
