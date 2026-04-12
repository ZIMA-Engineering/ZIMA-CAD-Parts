#ifndef SECRETSTORE_H
#define SECRETSTORE_H

#include <QObject>
#include <QString>

class SecretStore : public QObject
{
    Q_OBJECT

public:
    explicit SecretStore(QObject *parent = 0)
        : QObject(parent)
    {
    }

    virtual ~SecretStore()
    {
    }

    virtual bool writeSecret(const QString &id, const QString &secret, QString *errorMessage) = 0;
    virtual bool readSecret(const QString &id, QString *secret, QString *errorMessage, bool *notFound = 0) = 0;
    virtual bool deleteSecret(const QString &id, QString *errorMessage, bool *notFound = 0) = 0;
};

#endif // SECRETSTORE_H
