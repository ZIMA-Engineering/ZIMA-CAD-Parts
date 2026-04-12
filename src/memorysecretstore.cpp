#include "memorysecretstore.h"

MemorySecretStore::MemorySecretStore(QObject *parent)
    : SecretStore(parent)
{
}

bool MemorySecretStore::writeSecret(const QString &id, const QString &secret, QString *errorMessage)
{
    Q_UNUSED(errorMessage)

    m_secrets[id] = secret;
    return true;
}

bool MemorySecretStore::readSecret(const QString &id, QString *secret, QString *errorMessage, bool *notFound)
{
    Q_UNUSED(errorMessage)

    const bool exists = m_secrets.contains(id);
    if (notFound)
        *notFound = !exists;

    if (!exists)
        return false;

    if (secret)
        *secret = m_secrets.value(id);
    return true;
}

bool MemorySecretStore::deleteSecret(const QString &id, QString *errorMessage, bool *notFound)
{
    Q_UNUSED(errorMessage)

    const bool exists = m_secrets.contains(id);
    if (notFound)
        *notFound = !exists;

    if (!exists)
        return false;

    m_secrets.remove(id);
    return true;
}
