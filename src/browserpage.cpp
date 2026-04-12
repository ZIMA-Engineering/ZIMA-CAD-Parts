/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "browserpage.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineScript>
#include <QWebEngineView>

#include "browserprofilemanager.h"
#include "passwordmanager.h"
#include "passwordmanagerbridge.h"

BrowserPage::BrowserPage(QObject *parent)
    : QWebEnginePage(BrowserProfileManager::instance()->profile(), parent),
      m_channel(new QWebChannel(this)),
      m_bridge(new PasswordManagerBridge(this, BrowserProfileManager::instance()->passwordManager(), this))
{
    m_channel->registerObject(QStringLiteral("zcpPasswordManager"), m_bridge);
    setWebChannel(m_channel, QWebEngineScript::ApplicationWorld);

    connect(this, SIGNAL(authenticationRequired(QUrl,QAuthenticator*)),
            this, SLOT(handleAuthenticationRequired(QUrl,QAuthenticator*)));
}

QWebEnginePage *BrowserPage::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type)

    QWebEngineView *ownerView = qobject_cast<QWebEngineView *>(parent());
    QDialog *dialog = new QDialog(ownerView ? ownerView->window() : 0);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Browser popup"));
    dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinMaxButtonsHint);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 0);

    QWebEngineView *popupView = new QWebEngineView(dialog);
    layout->addWidget(popupView);

    BrowserPage *popupPage = new BrowserPage(popupView);
    popupView->setPage(popupPage);

    if (ownerView)
        dialog->resize(ownerView->size());
    else
        dialog->resize(1024, 768);

    connect(popupPage, SIGNAL(windowCloseRequested()),
            dialog, SLOT(close()));

    dialog->show();
    return popupPage;
}

void BrowserPage::handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator)
{
    QWebEngineView *ownerView = qobject_cast<QWebEngineView *>(parent());
    BrowserProfileManager::instance()->passwordManager()->handleHttpAuthentication(ownerView ? ownerView->window() : 0,
                                                                                  requestUrl,
                                                                                  authenticator);
}
