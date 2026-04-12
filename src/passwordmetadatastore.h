#ifndef PASSWORDMETADATASTORE_H
#define PASSWORDMETADATASTORE_H

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QList>

struct PasswordMetadata
{
    QString id;
    QString type;
    QString origin;
    QString username;
    QString realm;
    QString actionOrigin;
    QString actionPath;
    QString usernameFieldName;
    QString usernameFieldId;
    QString passwordFieldName;
    QString passwordFieldId;
    QDateTime createdAtUtc;
    QDateTime updatedAtUtc;
    QDateTime lastUsedAtUtc;
};

class PasswordMetadataStore : public QObject
{
    Q_OBJECT

public:
    explicit PasswordMetadataStore(QObject *parent = 0);

    QList<PasswordMetadata> allCredentials() const;
    PasswordMetadata credential(const QString &id, bool *found = 0) const;
    void saveCredential(const PasswordMetadata &metadata);
    void removeCredential(const QString &id);
};

#endif // PASSWORDMETADATASTORE_H
