#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>

class PasswordMetadataStore;
class QAuthenticator;
class QWebEngineProfile;
class SecretStore;
class QWidget;

class PasswordManager : public QObject
{
    Q_OBJECT

public:
    explicit PasswordManager(QWebEngineProfile *profile, QObject *parent = 0);

    static QString canonicalOrigin(const QUrl &url);
    static bool isSupportedCredentialOrigin(const QUrl &url);

    PasswordMetadataStore *metadataStore() const;
    SecretStore *secretStore() const;
    QWebEngineProfile *profile() const;

    void handleHttpAuthentication(QWidget *parent, const QUrl &requestUrl, QAuthenticator *authenticator);
    QVariantMap buildAutofillResponse(const QVariantMap &request, QWidget *parent);
    void handleFormSubmitted(QWidget *parent, const QVariantMap &submission);

private:
    struct HttpAuthLookupResult {
        bool found;
        QString id;
        QString username;
        QString password;
    };

    HttpAuthLookupResult lookupHttpAuthCredential(const QString &origin,
                                                  const QString &realm,
                                                  const QString &preferredUsername,
                                                  QWidget *parent);
    void saveHttpAuthCredential(const QString &origin,
                                const QString &realm,
                                const QString &username,
                                const QString &password,
                                QWidget *parent);
    void installFormScript();
    void warnSecretStoreFailure(QWidget *parent, const QString &details);

    QWebEngineProfile *m_profile;
    PasswordMetadataStore *m_metadataStore;
    SecretStore *m_secretStore;
    bool m_usingMemorySecretStore;
    bool m_secretStoreWarningShown;
};

#endif // PASSWORDMANAGER_H
