#include "webauthenticationdialog.h"
#include "ui_webauthenticationdialog.h"

WebAuthenticationDialog::WebAuthenticationDialog(QAuthenticator *authenticator, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WebAuthenticationDialog),
	m_authenticator(authenticator)
{
	ui->setupUi(this);

	ui->realmLabel->setText(authenticator->realm());
}

WebAuthenticationDialog::~WebAuthenticationDialog()
{
	delete ui;
}

void WebAuthenticationDialog::authenticate()
{
	m_authenticator->setUser(ui->usernameLineEdit->text());
	m_authenticator->setPassword(ui->passwordLineEdit->text());
}
