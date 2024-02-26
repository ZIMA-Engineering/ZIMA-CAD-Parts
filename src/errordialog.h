#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QHash>

namespace Ui {
class ErrorDialog;
}

typedef QHash<QString,QString> ErrorsMap;
typedef QHashIterator<QString,QString> ErrorsMapIterator;

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = 0);
    ~ErrorDialog();
    void setErrors(const QString &label, const ErrorsMap &errors);

private:
    Ui::ErrorDialog *ui;
};

#endif // ERRORDIALOG_H
