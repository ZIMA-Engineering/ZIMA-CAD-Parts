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

void ErrorDialog::setErrors(const QString &label, const ErrorsMap &errors)
{
    ui->label->setText(label);

    QString str = "<html><body><dl>";
    ErrorsMapIterator it(errors);
    while (it.hasNext())
    {
        it.next();
        str += "<dt>" + it.key() + ":</dt>";
        str += "<dd>" + it.value() + ":</dd>";
    }
    str += "</dl></body></html>";

    ui->textEdit->setHtml(str);
}

