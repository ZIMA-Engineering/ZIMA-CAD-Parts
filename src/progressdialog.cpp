#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

QLabel *ProgressDialog::label()
{
    return ui->progressLabel;
}

QProgressBar *ProgressDialog::progressBar()
{
    return ui->progressBar;
}
