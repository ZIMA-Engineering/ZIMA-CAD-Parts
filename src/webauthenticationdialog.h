#ifndef WEBAUTHENTICATIONDIALOG_H
#define WEBAUTHENTICATIONDIALOG_H

#include <QDialog>

namespace Ui {
class WebAuthenticationDialog;
}

class WebAuthenticationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WebAuthenticationDialog(const QString &site,
                                     const QString &realm,
                                     const QString &initialUsername,
                                     const QString &initialPassword,
                                     bool initialRememberCredentials,
                                     QWidget *parent = 0);
    ~WebAuthenticationDialog();

    QString username() const;
    QString password() const;
    bool rememberCredentials() const;

private:
    Ui::WebAuthenticationDialog *ui;
};

#endif // WEBAUTHENTICATIONDIALOG_H
