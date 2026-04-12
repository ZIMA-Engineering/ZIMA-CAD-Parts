#include "webauthenticationdialog.h"
#include "ui_webauthenticationdialog.h"

WebAuthenticationDialog::WebAuthenticationDialog(const QString &site,
                                                 const QString &realm,
                                                 const QString &initialUsername,
                                                 const QString &initialPassword,
                                                 bool initialRememberCredentials,
                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebAuthenticationDialog)
{
    ui->setupUi(this);

    ui->siteLabel->setText(site);
    ui->realmLabel->setText(realm);
    ui->usernameLineEdit->setText(initialUsername);
    ui->passwordLineEdit->setText(initialPassword);
    ui->rememberCredentialsCheckBox->setChecked(initialRememberCredentials);

    if (initialUsername.isEmpty())
        ui->usernameLineEdit->setFocus();
    else
        ui->passwordLineEdit->setFocus();
}

WebAuthenticationDialog::~WebAuthenticationDialog()
{
    delete ui;
}

QString WebAuthenticationDialog::username() const
{
    return ui->usernameLineEdit->text();
}

QString WebAuthenticationDialog::password() const
{
    return ui->passwordLineEdit->text();
}

bool WebAuthenticationDialog::rememberCredentials() const
{
    return ui->rememberCredentialsCheckBox->isChecked();
}
