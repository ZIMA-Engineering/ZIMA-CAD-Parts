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

#include "savedpasswordsdialog.h"
#include "ui_savedpasswordsdialog.h"

#include <algorithm>

#include <QAbstractItemView>
#include <QClipboard>
#include <QGuiApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QUrl>

#include "browserprofilemanager.h"
#include "passwordmanager.h"
#include "secretstore.h"

namespace {

QString displaySite(const PasswordMetadata &metadata)
{
    const QUrl url(metadata.origin);
    return url.host().isEmpty() ? metadata.origin : url.host();
}

QString displayType(const PasswordMetadata &metadata)
{
    return metadata.type == QStringLiteral("http-auth")
            ? QObject::tr("HTTP authentication")
            : QObject::tr("Website");
}

QString fieldIdentifier(const QString &name, const QString &id)
{
    if (!name.isEmpty())
        return name;
    if (!id.isEmpty())
        return id;
    return QString();
}

QString displayDetail(const PasswordMetadata &metadata)
{
    if (metadata.type == QStringLiteral("http-auth"))
        return metadata.realm;

    QString detail = metadata.actionPath.isEmpty() ? QStringLiteral("/") : metadata.actionPath;
    const QString usernameField = fieldIdentifier(metadata.usernameFieldName, metadata.usernameFieldId);
    const QString passwordField = fieldIdentifier(metadata.passwordFieldName, metadata.passwordFieldId);

    if (!usernameField.isEmpty() && !passwordField.isEmpty())
        detail += QObject::tr(" (%1/%2)").arg(usernameField, passwordField);

    return detail;
}

QString displayLastUsed(const PasswordMetadata &metadata)
{
    if (!metadata.lastUsedAtUtc.isValid())
        return QObject::tr("Never");

    return QLocale().toString(metadata.lastUsedAtUtc.toLocalTime(), QLocale::ShortFormat);
}

bool matchesFilter(const PasswordMetadata &metadata, const QString &filterText)
{
    if (filterText.isEmpty())
        return true;

    const QStringList haystack = QStringList()
            << displaySite(metadata)
            << metadata.origin
            << metadata.username
            << displayType(metadata)
            << displayDetail(metadata)
            << metadata.realm;

    for (const QString &value : haystack) {
        if (value.contains(filterText, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

void setTableItem(QTableWidget *table, int row, int column, const QString &text, const QString &credentialId)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setData(Qt::UserRole, credentialId);
    table->setItem(row, column, item);
}

}

SavedPasswordsDialog::SavedPasswordsDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::SavedPasswordsDialog)
{
    ui->setupUi(this);

    ui->passwordsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->passwordsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->passwordsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->passwordsTableWidget->setAlternatingRowColors(true);
    ui->passwordsTableWidget->verticalHeader()->setVisible(false);
    ui->passwordsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->passwordsTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->passwordsTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->passwordsTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->passwordsTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    connect(ui->searchLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(refreshCredentials()));
    connect(ui->passwordsTableWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateSelection()));
    connect(ui->revealButton, SIGNAL(clicked()),
            this, SLOT(toggleReveal()));
    connect(ui->copyUsernameButton, SIGNAL(clicked()),
            this, SLOT(copyUsername()));
    connect(ui->copyPasswordButton, SIGNAL(clicked()),
            this, SLOT(copyPassword()));
    connect(ui->deleteButton, SIGNAL(clicked()),
            this, SLOT(deleteSelected()));
    connect(ui->deleteAllButton, SIGNAL(clicked()),
            this, SLOT(deleteAll()));
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    refreshCredentials();
}

SavedPasswordsDialog::~SavedPasswordsDialog()
{
    delete ui;
}

PasswordManager *SavedPasswordsDialog::passwordManager() const
{
    return BrowserProfileManager::instance()->passwordManager();
}

PasswordMetadata SavedPasswordsDialog::selectedCredential(bool *found) const
{
    PasswordMetadata metadata;
    PasswordManager *manager = passwordManager();
    if (!manager) {
        if (found)
            *found = false;
        return metadata;
    }

    const QString id = currentCredentialId();
    if (id.isEmpty()) {
        if (found)
            *found = false;
        return metadata;
    }

    return manager->metadataStore()->credential(id, found);
}

QString SavedPasswordsDialog::currentCredentialId() const
{
    QTableWidgetItem *item = ui->passwordsTableWidget->currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

bool SavedPasswordsDialog::removeCredential(const PasswordMetadata &metadata, QString *errorMessage)
{
    PasswordManager *manager = passwordManager();
    if (!manager || metadata.id.isEmpty())
        return false;

    bool notFound = false;
    QString localError;
    if (!manager->secretStore()->deleteSecret(metadata.id, &localError, &notFound) && !notFound) {
        if (errorMessage)
            *errorMessage = localError;
        return false;
    }

    manager->metadataStore()->removeCredential(metadata.id);
    return true;
}

bool SavedPasswordsDialog::readCredentialSecret(const PasswordMetadata &metadata,
                                                QString *secret,
                                                QString *errorMessage)
{
    PasswordManager *manager = passwordManager();
    if (!manager || metadata.id.isEmpty())
        return false;

    bool notFound = false;
    QString localError;
    if (!manager->secretStore()->readSecret(metadata.id, secret, &localError, &notFound)) {
        if (errorMessage) {
            *errorMessage = notFound
                    ? tr("The password for this entry is no longer available in the system credential store.")
                    : localError;
        }
        return false;
    }

    return true;
}

void SavedPasswordsDialog::refreshCredentials()
{
    repopulateTable(currentCredentialId());
}

void SavedPasswordsDialog::repopulateTable(const QString &selectedId)
{
    PasswordManager *manager = passwordManager();
    const QList<PasswordMetadata> allCredentials = manager
            ? manager->metadataStore()->allCredentials()
            : QList<PasswordMetadata>();
    QList<PasswordMetadata> visibleCredentials;
    const QString filterText = ui->searchLineEdit->text().trimmed();

    for (const PasswordMetadata &metadata : allCredentials) {
        if (matchesFilter(metadata, filterText))
            visibleCredentials.append(metadata);
    }

    std::sort(visibleCredentials.begin(), visibleCredentials.end(),
              [](const PasswordMetadata &left, const PasswordMetadata &right) {
        if (left.lastUsedAtUtc != right.lastUsedAtUtc)
            return left.lastUsedAtUtc > right.lastUsedAtUtc;
        return left.updatedAtUtc > right.updatedAtUtc;
    });

    ui->passwordsTableWidget->setRowCount(visibleCredentials.count());

    int rowToSelect = -1;
    for (int row = 0; row < visibleCredentials.count(); ++row) {
        const PasswordMetadata &metadata = visibleCredentials.at(row);
        setTableItem(ui->passwordsTableWidget, row, 0, displaySite(metadata), metadata.id);
        setTableItem(ui->passwordsTableWidget, row, 1, metadata.username, metadata.id);
        setTableItem(ui->passwordsTableWidget, row, 2, displayType(metadata), metadata.id);
        setTableItem(ui->passwordsTableWidget, row, 3, displayDetail(metadata), metadata.id);
        setTableItem(ui->passwordsTableWidget, row, 4, displayLastUsed(metadata), metadata.id);

        if (!selectedId.isEmpty() && metadata.id == selectedId)
            rowToSelect = row;
    }

    if (rowToSelect < 0 && ui->passwordsTableWidget->rowCount() > 0)
        rowToSelect = 0;

    if (rowToSelect >= 0) {
        ui->passwordsTableWidget->selectRow(rowToSelect);
    } else {
        ui->passwordsTableWidget->clearSelection();
        clearDetails();
    }

    ui->deleteAllButton->setEnabled(!allCredentials.isEmpty());
    updateSelection();
}

void SavedPasswordsDialog::updateSelection()
{
    bool found = false;
    const PasswordMetadata metadata = selectedCredential(&found);
    if (!found) {
        clearDetails();
        return;
    }

    ui->siteLineEdit->setText(metadata.origin);
    ui->usernameLineEdit->setText(metadata.username);
    hidePassword();

    ui->revealButton->setEnabled(true);
    ui->copyUsernameButton->setEnabled(true);
    ui->copyPasswordButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
}

void SavedPasswordsDialog::clearDetails()
{
    m_revealedCredentialId.clear();
    ui->siteLineEdit->clear();
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->revealButton->setText(tr("Reveal"));
    ui->revealButton->setEnabled(false);
    ui->copyUsernameButton->setEnabled(false);
    ui->copyPasswordButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
}

void SavedPasswordsDialog::hidePassword()
{
    m_revealedCredentialId.clear();
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->passwordLineEdit->setText(QStringLiteral("placeholder"));
    ui->revealButton->setText(tr("Reveal"));
}

void SavedPasswordsDialog::toggleReveal()
{
    bool found = false;
    const PasswordMetadata metadata = selectedCredential(&found);
    if (!found)
        return;

    if (m_revealedCredentialId == metadata.id) {
        hidePassword();
        return;
    }

    QString secret;
    QString errorMessage;
    if (!readCredentialSecret(metadata, &secret, &errorMessage)) {
        QMessageBox::warning(this,
                             tr("Unable to reveal password"),
                             errorMessage);
        hidePassword();
        return;
    }

    m_revealedCredentialId = metadata.id;
    ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);
    ui->passwordLineEdit->setText(secret);
    ui->revealButton->setText(tr("Hide"));
}

void SavedPasswordsDialog::copyUsername()
{
    bool found = false;
    const PasswordMetadata metadata = selectedCredential(&found);
    if (!found)
        return;

    if (QGuiApplication::clipboard())
        QGuiApplication::clipboard()->setText(metadata.username);
}

void SavedPasswordsDialog::copyPassword()
{
    bool found = false;
    const PasswordMetadata metadata = selectedCredential(&found);
    if (!found)
        return;

    QString secret;
    QString errorMessage;
    if (!readCredentialSecret(metadata, &secret, &errorMessage)) {
        QMessageBox::warning(this,
                             tr("Unable to copy password"),
                             errorMessage);
        return;
    }

    if (QGuiApplication::clipboard())
        QGuiApplication::clipboard()->setText(secret);
}

void SavedPasswordsDialog::deleteSelected()
{
    bool found = false;
    const PasswordMetadata metadata = selectedCredential(&found);
    if (!found)
        return;

    QString errorMessage;
    if (!removeCredential(metadata, &errorMessage)) {
        QMessageBox::warning(this,
                             tr("Unable to delete password"),
                             tr("The selected entry could not be removed from the system credential store.\n\n%1")
                             .arg(errorMessage));
        return;
    }

    refreshCredentials();
}

void SavedPasswordsDialog::deleteAll()
{
    PasswordManager *manager = passwordManager();
    if (!manager)
        return;

    const QList<PasswordMetadata> credentials = manager->metadataStore()->allCredentials();
    if (credentials.isEmpty())
        return;

    if (QMessageBox::question(this,
                              tr("Delete all saved passwords"),
                              tr("Delete all saved passwords?")) != QMessageBox::Yes) {
        return;
    }

    int failedCount = 0;
    for (const PasswordMetadata &metadata : credentials) {
        QString errorMessage;
        if (!removeCredential(metadata, &errorMessage))
            ++failedCount;
    }

    refreshCredentials();

    if (failedCount > 0) {
        QMessageBox::warning(this,
                             tr("Unable to delete all passwords"),
                             tr("%1 saved password entries could not be removed from the system credential store.")
                             .arg(failedCount));
    }
}
