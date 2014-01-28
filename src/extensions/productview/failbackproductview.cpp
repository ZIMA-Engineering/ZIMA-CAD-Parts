#include "failbackproductview.h"
#include "ui_failbackproductview.h"

FailbackProductView::FailbackProductView(QWidget *parent) :
	AbstractProductView(parent),
	ui(new Ui::FailbackProductView)
{
	ui->setupUi(this);
}

FailbackProductView::~FailbackProductView()
{
	delete ui;
}

QString FailbackProductView::title()
{
	return tr("Common Document");
}

QList<File::FileTypes> FailbackProductView::canHandle()
{
	return QList<File::FileTypes>();
}

bool FailbackProductView::handle(File *f)
{
	QString html = "<html><body><table width=\"100%\">";
	html += "<tr><th>" + tr("Property") + "</th><th>" + tr("Value") + "</th></tr>";
	html += "<tr><td>" + tr("Name") + "</td><td>" + f->baseName() + "</td></tr>";
	html += "<tr><td>" + tr("Checked") + "</td><td>" + (f->isChecked ? "Y":"N") + "</td></tr>";
	html += "<tr><td>" + tr("Last Modified") + "</td><td>" + f->lastModified.toString() + "</td></tr>";
	html += "<tr><td>" + tr("Size") + "</td><td>" + QString("%1").arg(f->size) + "</td></tr>";
	html += "<tr><td>" + tr("Version") + "</td><td>" + QString("%1").arg(f->version) + "</td></tr>";
	html += "</table></body></html>";

	ui->textBrowser->setHtml(html);

	return true;
}
