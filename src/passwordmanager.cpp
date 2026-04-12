#include "passwordmanager.h"

#include <algorithm>

#include <QAuthenticator>
#include <QDateTime>
#include <QFile>
#include <QHash>
#include <QMessageBox>
#include <QPushButton>
#include <QUuid>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

#include "memorysecretstore.h"
#include "passwordmetadatastore.h"
#include "qtkeychainsecretstore.h"
#include "settings.h"
#include "webauthenticationdialog.h"

namespace {

struct FormRequestData
{
    QString requestId;
    QString origin;
    QString actionOrigin;
    QString actionPath;
    QString usernameFieldName;
    QString usernameFieldId;
    QString passwordFieldName;
    QString passwordFieldId;
    QString username;
    QString password;
};

struct RankedFormCredential
{
    PasswordMetadata metadata;
    int score = -1;
};

QString normalizedActionPath(const QString &path)
{
    return path.isEmpty() ? QStringLiteral("/") : path;
}

QString displayHost(const QString &origin)
{
    const QUrl url(origin);
    return url.host().isEmpty() ? origin : url.host();
}

QString canonicalStringOrigin(const QString &urlString)
{
    return PasswordManager::canonicalOrigin(QUrl(urlString));
}

bool fieldsMatch(const QString &requestName,
                 const QString &requestId,
                 const QString &storedName,
                 const QString &storedId)
{
    if (!requestName.isEmpty() && !storedName.isEmpty())
        return requestName == storedName;

    if (!requestId.isEmpty() && !storedId.isEmpty())
        return requestId == storedId;

    return requestName == storedName && requestId == storedId;
}

FormRequestData formRequestFromMap(const QVariantMap &payload)
{
    FormRequestData data;
    data.requestId = payload.value(QStringLiteral("requestId")).toString();
    data.origin = canonicalStringOrigin(payload.value(QStringLiteral("origin")).toString());
    data.actionOrigin = canonicalStringOrigin(payload.value(QStringLiteral("actionOrigin")).toString());
    data.actionPath = normalizedActionPath(payload.value(QStringLiteral("actionPath")).toString());
    data.usernameFieldName = payload.value(QStringLiteral("usernameFieldName")).toString();
    data.usernameFieldId = payload.value(QStringLiteral("usernameFieldId")).toString();
    data.passwordFieldName = payload.value(QStringLiteral("passwordFieldName")).toString();
    data.passwordFieldId = payload.value(QStringLiteral("passwordFieldId")).toString();
    data.username = payload.value(QStringLiteral("usernameValue"),
                                  payload.value(QStringLiteral("username"))).toString();
    data.password = payload.value(QStringLiteral("password")).toString();
    return data;
}

bool isFormRequestValid(const FormRequestData &data)
{
    return !data.origin.isEmpty()
            && (!data.usernameFieldName.isEmpty() || !data.usernameFieldId.isEmpty())
            && (!data.passwordFieldName.isEmpty() || !data.passwordFieldId.isEmpty());
}

int autofillScore(const PasswordMetadata &metadata, const FormRequestData &request)
{
    if (metadata.type != QStringLiteral("html-form") || metadata.origin != request.origin)
        return -1;

    if (!fieldsMatch(request.usernameFieldName, request.usernameFieldId,
                     metadata.usernameFieldName, metadata.usernameFieldId)
            || !fieldsMatch(request.passwordFieldName, request.passwordFieldId,
                            metadata.passwordFieldName, metadata.passwordFieldId)) {
        return -1;
    }

    int score = 100;
    if (metadata.actionPath == request.actionPath)
        score = 200;
    if (!request.actionOrigin.isEmpty()
            && metadata.actionOrigin == request.actionOrigin
            && metadata.actionPath == request.actionPath) {
        score = 300;
    }
    if (!request.username.isEmpty() && metadata.username == request.username)
        score += 10;

    return score;
}

bool isBetterRankedFormCredential(const RankedFormCredential &candidate,
                                  const RankedFormCredential &current)
{
    return candidate.score > current.score
            || (candidate.score == current.score
                && candidate.metadata.lastUsedAtUtc > current.metadata.lastUsedAtUtc)
            || (candidate.score == current.score
                && candidate.metadata.lastUsedAtUtc == current.metadata.lastUsedAtUtc
                && candidate.metadata.updatedAtUtc > current.metadata.updatedAtUtc);
}

bool rankedFormCredentialLessThan(const RankedFormCredential &left,
                                  const RankedFormCredential &right,
                                  const QString &requestedUsername)
{
    const bool leftExactMatch = !requestedUsername.isEmpty()
            && left.metadata.username == requestedUsername;
    const bool rightExactMatch = !requestedUsername.isEmpty()
            && right.metadata.username == requestedUsername;

    if (leftExactMatch != rightExactMatch)
        return leftExactMatch;

    if (left.score != right.score)
        return left.score > right.score;

    if (left.metadata.lastUsedAtUtc != right.metadata.lastUsedAtUtc)
        return left.metadata.lastUsedAtUtc > right.metadata.lastUsedAtUtc;

    if (left.metadata.updatedAtUtc != right.metadata.updatedAtUtc)
        return left.metadata.updatedAtUtc > right.metadata.updatedAtUtc;

    return left.metadata.username < right.metadata.username;
}

QList<RankedFormCredential> matchingFormCredentials(const FormRequestData &request,
                                                    const QList<PasswordMetadata> &credentials)
{
    QHash<QString, RankedFormCredential> bestByUsername;

    for (const PasswordMetadata &metadata : credentials) {
        const int score = autofillScore(metadata, request);
        if (score < 0)
            continue;

        RankedFormCredential candidate;
        candidate.metadata = metadata;
        candidate.score = score;

        if (!bestByUsername.contains(metadata.username)
                || isBetterRankedFormCredential(candidate,
                                                bestByUsername.value(metadata.username))) {
            bestByUsername.insert(metadata.username, candidate);
        }
    }

    QList<RankedFormCredential> candidates = bestByUsername.values();
    std::sort(candidates.begin(),
              candidates.end(),
              [&request](const RankedFormCredential &left,
                         const RankedFormCredential &right) {
        return rankedFormCredentialLessThan(left, right, request.username);
    });
    return candidates;
}

bool isSameStoredForm(const PasswordMetadata &metadata, const FormRequestData &request)
{
    return metadata.type == QStringLiteral("html-form")
            && metadata.origin == request.origin
            && metadata.username == request.username
            && metadata.actionOrigin == request.actionOrigin
            && metadata.actionPath == request.actionPath
            && fieldsMatch(request.usernameFieldName, request.usernameFieldId,
                           metadata.usernameFieldName, metadata.usernameFieldId)
            && fieldsMatch(request.passwordFieldName, request.passwordFieldId,
                           metadata.passwordFieldName, metadata.passwordFieldId);
}

bool askToStorePassword(QWidget *parent, bool updateExisting, const QString &origin, const QString &username)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(updateExisting
                       ? QObject::tr("Update saved password")
                       : QObject::tr("Save password"));
    box.setText(updateExisting
                ? QObject::tr("Update saved password for %1?").arg(displayHost(origin))
                : QObject::tr("Save password for %1?").arg(displayHost(origin)));
    box.setInformativeText(QObject::tr("Username: %1").arg(username));

    QAbstractButton *acceptButton = box.addButton(updateExisting
                                                  ? QObject::tr("Update")
                                                  : QObject::tr("Save"),
                                                  QMessageBox::AcceptRole);
    box.addButton(QObject::tr("Not now"), QMessageBox::RejectRole);
    box.exec();

    return box.clickedButton() == acceptButton;
}

}

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

    installFormScript();
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

QWebEngineProfile *PasswordManager::profile() const
{
    return m_profile;
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

QVariantMap PasswordManager::buildAutofillResponse(const QVariantMap &request, QWidget *parent)
{
    FormRequestData formRequest = formRequestFromMap(request);
    QVariantMap response;
    response.insert(QStringLiteral("requestId"), formRequest.requestId);

    if (!Settings::get()->BrowserAutoFillPasswords)
        return response;

    if (!isFormRequestValid(formRequest) || !isSupportedCredentialOrigin(QUrl(formRequest.origin)))
        return response;

    const QList<RankedFormCredential> candidates = matchingFormCredentials(formRequest,
                                                                           m_metadataStore->allCredentials());
    if (candidates.isEmpty())
        return response;

    QVariantList availableUsernames;
    for (const RankedFormCredential &candidate : candidates)
        availableUsernames.append(candidate.metadata.username);

    response.insert(QStringLiteral("availableUsernames"), availableUsernames);
    response.insert(QStringLiteral("savedAccountsLabel"),
                    tr("Saved accounts (%1)").arg(candidates.count()));
    response.insert(QStringLiteral("chooseAccountLabel"),
                    tr("Use saved account"));
    response.insert(QStringLiteral("chooseAccountPlaceholder"),
                    tr("Choose account"));

    const RankedFormCredential *selected = 0;
    if (formRequest.username.isEmpty()) {
        selected = &candidates.first();
    } else {
        for (const RankedFormCredential &candidate : candidates) {
            if (candidate.metadata.username == formRequest.username) {
                selected = &candidate;
                break;
            }
        }
    }

    if (!selected)
        return response;

    QString secret;
    QString errorMessage;
    bool notFound = false;
    if (!m_secretStore->readSecret(selected->metadata.id, &secret, &errorMessage, &notFound)) {
        if (!notFound)
            warnSecretStoreFailure(parent, errorMessage);
        return response;
    }

    response.insert(QStringLiteral("username"), selected->metadata.username);
    response.insert(QStringLiteral("password"), secret);
    return response;
}

void PasswordManager::handleFormSubmitted(QWidget *parent, const QVariantMap &submission)
{
    if (!Settings::get()->BrowserSaveFormPasswords)
        return;

    FormRequestData formRequest = formRequestFromMap(submission);
    if (!isFormRequestValid(formRequest)
            || !isSupportedCredentialOrigin(QUrl(formRequest.origin))
            || formRequest.username.isEmpty()
            || formRequest.password.isEmpty()) {
        return;
    }

    PasswordMetadata selected;
    bool selectedFound = false;

    const QList<PasswordMetadata> credentials = m_metadataStore->allCredentials();
    for (const PasswordMetadata &metadata : credentials) {
        if (!isSameStoredForm(metadata, formRequest))
            continue;

        if (!selectedFound || metadata.updatedAtUtc > selected.updatedAtUtc) {
            selected = metadata;
            selectedFound = true;
        }
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();

    if (selectedFound) {
        QString existingSecret;
        QString errorMessage;
        bool notFound = false;
        if (!m_secretStore->readSecret(selected.id, &existingSecret, &errorMessage, &notFound) && !notFound) {
            warnSecretStoreFailure(parent, errorMessage);
            return;
        }

        if (!notFound && existingSecret == formRequest.password) {
            selected.lastUsedAtUtc = now;
            m_metadataStore->saveCredential(selected);
            return;
        }

        if (!askToStorePassword(parent, true, formRequest.origin, formRequest.username))
            return;

        if (!m_secretStore->writeSecret(selected.id, formRequest.password, &errorMessage)) {
            warnSecretStoreFailure(parent, errorMessage);
            return;
        }

        selected.updatedAtUtc = now;
        selected.lastUsedAtUtc = now;
        selected.username = formRequest.username;
        selected.actionOrigin = formRequest.actionOrigin;
        selected.actionPath = formRequest.actionPath;
        selected.usernameFieldName = formRequest.usernameFieldName;
        selected.usernameFieldId = formRequest.usernameFieldId;
        selected.passwordFieldName = formRequest.passwordFieldName;
        selected.passwordFieldId = formRequest.passwordFieldId;
        m_metadataStore->saveCredential(selected);
        return;
    }

    if (!askToStorePassword(parent, false, formRequest.origin, formRequest.username))
        return;

    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString errorMessage;
    if (!m_secretStore->writeSecret(id, formRequest.password, &errorMessage)) {
        warnSecretStoreFailure(parent, errorMessage);
        return;
    }

    PasswordMetadata metadata;
    metadata.id = id;
    metadata.type = QStringLiteral("html-form");
    metadata.origin = formRequest.origin;
    metadata.username = formRequest.username;
    metadata.actionOrigin = formRequest.actionOrigin;
    metadata.actionPath = formRequest.actionPath;
    metadata.usernameFieldName = formRequest.usernameFieldName;
    metadata.usernameFieldId = formRequest.usernameFieldId;
    metadata.passwordFieldName = formRequest.passwordFieldName;
    metadata.passwordFieldId = formRequest.passwordFieldId;
    metadata.createdAtUtc = now;
    metadata.updatedAtUtc = now;
    metadata.lastUsedAtUtc = now;
    m_metadataStore->saveCredential(metadata);
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

void PasswordManager::installFormScript()
{
    if (!m_profile)
        return;

    QFile webChannelFile(QStringLiteral(":/qtwebchannel/qwebchannel.js"));
    QFile passwordManagerFile(QStringLiteral(":/data/password_manager.js"));
    if (!webChannelFile.open(QIODevice::ReadOnly) || !passwordManagerFile.open(QIODevice::ReadOnly))
        return;

    QWebEngineScript script;
    script.setName(QStringLiteral("zcp_password_manager"));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setRunsOnSubFrames(false);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    script.setSourceCode(QString::fromUtf8(webChannelFile.readAll())
                         + QStringLiteral("\n")
                         + QString::fromUtf8(passwordManagerFile.readAll()));

    m_profile->scripts()->insert(script);
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
