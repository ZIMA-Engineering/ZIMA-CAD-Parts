#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

class QLabel;
class QProgressBar;

namespace Ui {
class ProgressDialog;
}

/*! Generic dialog with progress bar and label.
 */
class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();
    QLabel* label();
    QProgressBar* progressBar();

private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
