#ifndef WEBAUTHENTICATIONDIALOG_H
#define WEBAUTHENTICATIONDIALOG_H

#include <QDialog>
#include <QAuthenticator>

namespace Ui {
class WebAuthenticationDialog;
}

class WebAuthenticationDialog : public QDialog
{
	Q_OBJECT

public:
	explicit WebAuthenticationDialog(QAuthenticator *authenticator, QWidget *parent = 0);
	~WebAuthenticationDialog();
	void authenticate();

private:
	Ui::WebAuthenticationDialog *ui;
	QAuthenticator *m_authenticator;
};

#endif // WEBAUTHENTICATIONDIALOG_H
