#ifndef PARTSDELETEDIALOG_H
#define PARTSDELETEDIALOG_H

#include <QDialog>

namespace Ui {
class PartsDeleteDialog;
}

class PartsDeleteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PartsDeleteDialog(QWidget *parent = nullptr);
    ~PartsDeleteDialog();

private:
    Ui::PartsDeleteDialog *ui;
};

#endif // PARTSDELETEDIALOG_H
