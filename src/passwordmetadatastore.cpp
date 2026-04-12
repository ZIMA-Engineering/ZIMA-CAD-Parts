#include "passwordmetadatastore.h"

#include <QSettings>

namespace {

QString credentialsRoot()
{
    return QStringLiteral("Browser/SavedCredentials");
}

QDateTime readUtcDateTime(const QVariant &value)
{
    return value.toDateTime().toUTC();
}

void writeMetadata(QSettings *settings, const PasswordMetadata &metadata)
{
    settings->setValue(QStringLiteral("Type"), metadata.type);
    settings->setValue(QStringLiteral("Origin"), metadata.origin);
    settings->setValue(QStringLiteral("Username"), metadata.username);
    settings->setValue(QStringLiteral("Realm"), metadata.realm);
    settings->setValue(QStringLiteral("ActionOrigin"), metadata.actionOrigin);
    settings->setValue(QStringLiteral("ActionPath"), metadata.actionPath);
    settings->setValue(QStringLiteral("UsernameFieldName"), metadata.usernameFieldName);
    settings->setValue(QStringLiteral("UsernameFieldId"), metadata.usernameFieldId);
    settings->setValue(QStringLiteral("PasswordFieldName"), metadata.passwordFieldName);
    settings->setValue(QStringLiteral("PasswordFieldId"), metadata.passwordFieldId);
    settings->setValue(QStringLiteral("CreatedAtUtc"), metadata.createdAtUtc.toUTC());
    settings->setValue(QStringLiteral("UpdatedAtUtc"), metadata.updatedAtUtc.toUTC());
    settings->setValue(QStringLiteral("LastUsedAtUtc"), metadata.lastUsedAtUtc.toUTC());
}

PasswordMetadata readMetadata(QSettings *settings, const QString &id)
{
    PasswordMetadata metadata;
    metadata.id = id;
    metadata.type = settings->value(QStringLiteral("Type")).toString();
    metadata.origin = settings->value(QStringLiteral("Origin")).toString();
    metadata.username = settings->value(QStringLiteral("Username")).toString();
    metadata.realm = settings->value(QStringLiteral("Realm")).toString();
    metadata.actionOrigin = settings->value(QStringLiteral("ActionOrigin")).toString();
    metadata.actionPath = settings->value(QStringLiteral("ActionPath")).toString();
    metadata.usernameFieldName = settings->value(QStringLiteral("UsernameFieldName")).toString();
    metadata.usernameFieldId = settings->value(QStringLiteral("UsernameFieldId")).toString();
    metadata.passwordFieldName = settings->value(QStringLiteral("PasswordFieldName")).toString();
    metadata.passwordFieldId = settings->value(QStringLiteral("PasswordFieldId")).toString();
    metadata.createdAtUtc = readUtcDateTime(settings->value(QStringLiteral("CreatedAtUtc")));
    metadata.updatedAtUtc = readUtcDateTime(settings->value(QStringLiteral("UpdatedAtUtc")));
    metadata.lastUsedAtUtc = readUtcDateTime(settings->value(QStringLiteral("LastUsedAtUtc")));
    return metadata;
}

}

PasswordMetadataStore::PasswordMetadataStore(QObject *parent)
    : QObject(parent)
{
}

QList<PasswordMetadata> PasswordMetadataStore::allCredentials() const
{
    QList<PasswordMetadata> credentials;
    QSettings settings;

    settings.beginGroup(credentialsRoot());
    const QStringList ids = settings.childGroups();
    for (const QString &id : ids) {
        settings.beginGroup(id);
        credentials.append(readMetadata(&settings, id));
        settings.endGroup();
    }
    settings.endGroup();

    return credentials;
}

PasswordMetadata PasswordMetadataStore::credential(const QString &id, bool *found) const
{
    PasswordMetadata metadata;
    QSettings settings;

    settings.beginGroup(credentialsRoot());
    const bool exists = settings.childGroups().contains(id);
    if (found)
        *found = exists;

    if (exists) {
        settings.beginGroup(id);
        metadata = readMetadata(&settings, id);
        settings.endGroup();
    }

    settings.endGroup();
    return metadata;
}

void PasswordMetadataStore::saveCredential(const PasswordMetadata &metadata)
{
    if (metadata.id.isEmpty())
        return;

    QSettings settings;
    settings.beginGroup(credentialsRoot());
    settings.beginGroup(metadata.id);
    writeMetadata(&settings, metadata);
    settings.endGroup();
    settings.endGroup();
}

void PasswordMetadataStore::removeCredential(const QString &id)
{
    if (id.isEmpty())
        return;

    QSettings settings;
    settings.remove(credentialsRoot() + QLatin1Char('/') + id);
}
