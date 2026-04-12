#ifndef QTKEYCHAINSECRETSTORE_H
#define QTKEYCHAINSECRETSTORE_H

#include "secretstore.h"

class QtKeychainSecretStore : public SecretStore
{
    Q_OBJECT

public:
    explicit QtKeychainSecretStore(const QString &serviceName, QObject *parent = 0);

    bool writeSecret(const QString &id, const QString &secret, QString *errorMessage);
    bool readSecret(const QString &id, QString *secret, QString *errorMessage, bool *notFound = 0);
    bool deleteSecret(const QString &id, QString *errorMessage, bool *notFound = 0);

private:
    QString makeKey(const QString &id) const;

    QString m_serviceName;
};

#endif // QTKEYCHAINSECRETSTORE_H
