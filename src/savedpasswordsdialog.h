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

#ifndef SAVEDPASSWORDSDIALOG_H
#define SAVEDPASSWORDSDIALOG_H

#include <QDialog>

#include "passwordmetadatastore.h"

namespace Ui {
class SavedPasswordsDialog;
}

class PasswordManager;

class SavedPasswordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SavedPasswordsDialog(QWidget *parent = 0);
    ~SavedPasswordsDialog();

private slots:
    void refreshCredentials();
    void updateSelection();
    void toggleReveal();
    void copyUsername();
    void copyPassword();
    void deleteSelected();
    void deleteAll();

private:
    PasswordManager *passwordManager() const;
    PasswordMetadata selectedCredential(bool *found = 0) const;
    QString currentCredentialId() const;
    bool removeCredential(const PasswordMetadata &metadata, QString *errorMessage = 0);
    bool readCredentialSecret(const PasswordMetadata &metadata, QString *secret, QString *errorMessage = 0);
    void repopulateTable(const QString &selectedId = QString());
    void clearDetails();
    void hidePassword();

    Ui::SavedPasswordsDialog *ui;
    QString m_revealedCredentialId;
};

#endif // SAVEDPASSWORDSDIALOG_H
