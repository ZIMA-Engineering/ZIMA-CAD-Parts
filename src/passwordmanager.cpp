#include "passwordmanager.h"

#include <QAuthenticator>
#include <QDateTime>
#include <QMessageBox>
#include <QUuid>

#include "memorysecretstore.h"
#include "passwordmetadatastore.h"
#include "qtkeychainsecretstore.h"
#include "settings.h"
#include "webauthenticationdialog.h"

PasswordManager::PasswordManager(QWebEngineProfile *profile, QObject *parent)
    : QObject(parent),
      m_profile(profile),
      m_metadataStore(new PasswordMetadataStore(this)),
      m_secretStore(0),
      m_usingMemorySecretStore(false),
      m_secretStoreWarningShown(false)
{
    const QString secretStoreMode = qEnvironmentVariable("ZCP_SECRET_STORE").trimmed().toLower();
    if (secretStoreMode == QStringLiteral("memory")) {
        m_secretStore = new MemorySecretStore(this);
        m_usingMemorySecretStore = true;
    } else {
        m_secretStore = new QtKeychainSecretStore(QStringLiteral("ZIMA-CAD-Parts"), this);
    }
}

QString PasswordManager::canonicalOrigin(const QUrl &url)
{
    if (!url.isValid() || url.host().isEmpty())
        return QString();

    const QString scheme = url.scheme().toLower();
    const QString host = url.host().toLower();
    const int port = url.port();

    const bool defaultPort = (port == -1)
            || (scheme == QStringLiteral("http") && port == 80)
            || (scheme == QStringLiteral("https") && port == 443);

    return defaultPort
            ? QStringLiteral("%1://%2").arg(scheme, host)
            : QStringLiteral("%1://%2:%3").arg(scheme, host).arg(port);
}

bool PasswordManager::isSupportedCredentialOrigin(const QUrl &url)
{
    const QString scheme = url.scheme().toLower();
    return scheme == QStringLiteral("http") || scheme == QStringLiteral("https");
}

PasswordMetadataStore *PasswordManager::metadataStore() const
{
    return m_metadataStore;
}

SecretStore *PasswordManager::secretStore() const
{
    return m_secretStore;
}

void PasswordManager::handleHttpAuthentication(QWidget *parent, const QUrl &requestUrl, QAuthenticator *authenticator)
{
    const QString origin = canonicalOrigin(requestUrl);
    const QString realm = authenticator ? authenticator->realm() : QString();
    const bool persistenceEnabled = Settings::get()->BrowserRememberHttpAuth
            && isSupportedCredentialOrigin(requestUrl)
            && !origin.isEmpty();

    HttpAuthLookupResult lookup;
    lookup.found = false;
    if (persistenceEnabled)
        lookup = lookupHttpAuthCredential(origin, realm, authenticator->user(), parent);

    WebAuthenticationDialog dialog(origin.isEmpty() ? requestUrl.toString() : origin,
                                   realm,
                                   lookup.found ? lookup.username : authenticator->user(),
                                   lookup.found ? lookup.password : authenticator->password(),
                                   lookup.found,
                                   parent);

    if (dialog.exec() != QDialog::Accepted)
        return;

    authenticator->setUser(dialog.username());
    authenticator->setPassword(dialog.password());

    if (persistenceEnabled
            && dialog.rememberCredentials()
            && !dialog.username().isEmpty()
            && !dialog.password().isEmpty()) {
        saveHttpAuthCredential(origin, realm, dialog.username(), dialog.password(), parent);
    }
}

PasswordManager::HttpAuthLookupResult PasswordManager::lookupHttpAuthCredential(const QString &origin,
                                                                                const QString &realm,
                                                                                const QString &preferredUsername,
                                                                                QWidget *parent)
{
    HttpAuthLookupResult result;
    result.found = false;

    PasswordMetadata selected;
    bool selectedFound = false;

    const QList<PasswordMetadata> credentials = m_metadataStore->allCredentials();
    for (const PasswordMetadata &metadata : credentials) {
        if (metadata.type != QStringLiteral("http-auth")
                || metadata.origin != origin
                || metadata.realm != realm) {
            continue;
        }

        const bool preferredMatch = !preferredUsername.isEmpty() && metadata.username == preferredUsername;
        const bool selectedPreferredMatch = !preferredUsername.isEmpty() && selected.username == preferredUsername;

        if (!selectedFound
                || (preferredMatch && !selectedPreferredMatch)
                || (preferredMatch == selectedPreferredMatch && metadata.lastUsedAtUtc > selected.lastUsedAtUtc)) {
            selected = metadata;
            selectedFound = true;
        }
    }

    if (!selectedFound)
        return result;

    QString secret;
    QString errorMessage;
    bool notFound = false;
    if (!m_secretStore->readSecret(selected.id, &secret, &errorMessage, &notFound)) {
        if (!notFound)
            warnSecretStoreFailure(parent, errorMessage);
        return result;
    }

    result.found = true;
    result.id = selected.id;
    result.username = selected.username;
    result.password = secret;
    return result;
}

void PasswordManager::saveHttpAuthCredential(const QString &origin,
                                             const QString &realm,
                                             const QString &username,
                                             const QString &password,
                                             QWidget *parent)
{
    PasswordMetadata selected;
    bool selectedFound = false;

    const QList<PasswordMetadata> credentials = m_metadataStore->allCredentials();
    for (const PasswordMetadata &metadata : credentials) {
        if (metadata.type != QStringLiteral("http-auth")
                || metadata.origin != origin
                || metadata.realm != realm
                || metadata.username != username) {
            continue;
        }

        if (!selectedFound || metadata.updatedAtUtc > selected.updatedAtUtc) {
            selected = metadata;
            selectedFound = true;
        }
    }

    const QString id = selectedFound
            ? selected.id
            : QUuid::createUuid().toString(QUuid::WithoutBraces);

    QString errorMessage;
    if (!m_secretStore->writeSecret(id, password, &errorMessage)) {
        warnSecretStoreFailure(parent, errorMessage);
        return;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    PasswordMetadata metadata = selectedFound ? selected : PasswordMetadata();
    metadata.id = id;
    metadata.type = QStringLiteral("http-auth");
    metadata.origin = origin;
    metadata.username = username;
    metadata.realm = realm;
    if (!metadata.createdAtUtc.isValid())
        metadata.createdAtUtc = now;
    metadata.updatedAtUtc = now;
    metadata.lastUsedAtUtc = now;

    m_metadataStore->saveCredential(metadata);
}

void PasswordManager::warnSecretStoreFailure(QWidget *parent, const QString &details)
{
    if (m_usingMemorySecretStore || m_secretStoreWarningShown)
        return;

    m_secretStoreWarningShown = true;
    const QString informativeText = details.isEmpty()
            ? tr("Saved credentials could not be accessed in the system credential store.")
            : tr("Saved credentials could not be accessed in the system credential store.\n\n%1").arg(details);

    QMessageBox::warning(parent,
                         tr("Credential storage unavailable"),
                         informativeText);
}
