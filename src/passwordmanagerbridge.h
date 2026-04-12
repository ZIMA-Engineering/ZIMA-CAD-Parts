#ifndef PASSWORDMANAGERBRIDGE_H
#define PASSWORDMANAGERBRIDGE_H

#include <QObject>
#include <QPointer>
#include <QVariantMap>

class BrowserPage;
class PasswordManager;

class PasswordManagerBridge : public QObject
{
    Q_OBJECT

public:
    explicit PasswordManagerBridge(BrowserPage *page, PasswordManager *manager, QObject *parent = 0);

public slots:
    void requestAutofill(const QVariantMap &request);
    void formSubmitted(const QVariantMap &submission);

signals:
    void autofillReady(const QVariantMap &response);

private:
    QPointer<BrowserPage> m_page;
    PasswordManager *m_manager;
};

#endif // PASSWORDMANAGERBRIDGE_H
