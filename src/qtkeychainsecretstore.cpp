#include "qtkeychainsecretstore.h"

#include <QEventLoop>

#include "keychain.h"

namespace {

bool waitForJob(QKeychain::Job *job)
{
    QEventLoop loop;
    job->setAutoDelete(false);
    QObject::connect(job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job->start();
    loop.exec();
    return job->error() == QKeychain::NoError;
}

}

QtKeychainSecretStore::QtKeychainSecretStore(const QString &serviceName, QObject *parent)
    : SecretStore(parent),
      m_serviceName(serviceName)
{
}

bool QtKeychainSecretStore::writeSecret(const QString &id, const QString &secret, QString *errorMessage)
{
    QKeychain::WritePasswordJob *job = new QKeychain::WritePasswordJob(m_serviceName, this);
    job->setKey(makeKey(id));
    job->setTextData(secret);

    const bool ok = waitForJob(job);
    if (!ok && errorMessage)
        *errorMessage = job->errorString();

    job->deleteLater();
    return ok;
}

bool QtKeychainSecretStore::readSecret(const QString &id, QString *secret, QString *errorMessage, bool *notFound)
{
    QKeychain::ReadPasswordJob *job = new QKeychain::ReadPasswordJob(m_serviceName, this);
    job->setKey(makeKey(id));

    const bool ok = waitForJob(job);
    if (notFound)
        *notFound = (job->error() == QKeychain::EntryNotFound);

    if (ok && secret)
        *secret = job->textData();
    else if (!ok && errorMessage)
        *errorMessage = job->errorString();

    job->deleteLater();
    return ok;
}

bool QtKeychainSecretStore::deleteSecret(const QString &id, QString *errorMessage, bool *notFound)
{
    QKeychain::DeletePasswordJob *job = new QKeychain::DeletePasswordJob(m_serviceName, this);
    job->setKey(makeKey(id));

    const bool ok = waitForJob(job);
    if (notFound)
        *notFound = (job->error() == QKeychain::EntryNotFound);

    if (!ok && errorMessage)
        *errorMessage = job->errorString();

    job->deleteLater();
    return ok;
}

QString QtKeychainSecretStore::makeKey(const QString &id) const
{
    return QStringLiteral("browser/credential/%1").arg(id);
}
