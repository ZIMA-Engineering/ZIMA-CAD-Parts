#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ErrorDialog)
{
	ui->setupUi(this);
}

ErrorDialog::~ErrorDialog()
{
	delete ui;
}

void ErrorDialog::setError(QString s)
{
	ui->label->setText(s);
}

void ErrorDialog::setText(QString s)
{
	ui->textEdit->setHtml(s);
}
