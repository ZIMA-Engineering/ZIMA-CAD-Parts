#ifndef DIRECTORYCREATOR_H
#define DIRECTORYCREATOR_H

#include <QObject>

#include "threadworker.h"

class DirectoryCreator : public QObject
{
    Q_OBJECT

public:
    DirectoryCreator(const QString &dir, const QString &name, QObject *parent = 0);
    void setPrototype(const QString &prototype);

public slots:
    void work();

private:
    QString m_dir;
    QString m_name;
    QString m_prototype;
};

class DirectoryCreatorThread : public ThreadWorker {
    Q_OBJECT

public:
    void setInfo(const QString &dir, const QString &name, const QString &prototype = "");
    void run();

private:
    QString m_dir;
    QString m_name;
    QString m_prototype;

    void recursiveCopy(const QString &src, const QString &dst);
};

#endif // DIRECTORYCREATOR_H
