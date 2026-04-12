#include "passwordmanagerbridge.h"

#include <QWebEngineView>

#include "browserpage.h"
#include "passwordmanager.h"

PasswordManagerBridge::PasswordManagerBridge(BrowserPage *page, PasswordManager *manager, QObject *parent)
    : QObject(parent),
      m_page(page),
      m_manager(manager)
{
}

void PasswordManagerBridge::requestAutofill(const QVariantMap &request)
{
    if (!m_manager) {
        emit autofillReady(QVariantMap());
        return;
    }

    QWebEngineView *ownerView = m_page ? qobject_cast<QWebEngineView *>(m_page->parent()) : 0;
    emit autofillReady(m_manager->buildAutofillResponse(request, ownerView ? ownerView->window() : 0));
}

void PasswordManagerBridge::formSubmitted(const QVariantMap &submission)
{
    if (!m_manager)
        return;

    QWebEngineView *ownerView = m_page ? qobject_cast<QWebEngineView *>(m_page->parent()) : 0;
    m_manager->handleFormSubmitted(ownerView ? ownerView->window() : 0, submission);
}
