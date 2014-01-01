#include "webdownloaddialog.h"
#include "ui_webdownloaddialog.h"

WebDownloadDialog::WebDownloadDialog(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::WebDownloadDialog)
{
	ui->setupUi(this);
}

WebDownloadDialog::~WebDownloadDialog()
{
	delete ui;
}
