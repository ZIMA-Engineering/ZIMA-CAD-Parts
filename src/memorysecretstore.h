#ifndef MEMORYSECRETSTORE_H
#define MEMORYSECRETSTORE_H

#include <QHash>

#include "secretstore.h"

class MemorySecretStore : public SecretStore
{
    Q_OBJECT

public:
    explicit MemorySecretStore(QObject *parent = 0);

    bool writeSecret(const QString &id, const QString &secret, QString *errorMessage);
    bool readSecret(const QString &id, QString *secret, QString *errorMessage, bool *notFound = 0);
    bool deleteSecret(const QString &id, QString *errorMessage, bool *notFound = 0);

private:
    QHash<QString, QString> m_secrets;
};

#endif // MEMORYSECRETSTORE_H
