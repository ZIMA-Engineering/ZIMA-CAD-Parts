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

FileTypeList FailbackProductView::canHandle()
{
    return FileTypeList();
}

bool FailbackProductView::handle(FileMetadata *f)
{
    QString lastModified = f->fileInfo.lastModified().toString();

    QString html = "<html><body><table width=\"100%\" border=\"0\">";
    html += "<tr><th>" + tr("Property") + "</th><th>" + tr("Value") + "</th></tr>";
    html += "<tr><td>" + tr("Name") + "</td><td>" + f->fileInfo.baseName() + "</td></tr>";
    html += "<tr><td>" + tr("Last Modified") + "</td><td>" + lastModified + "</td></tr>";
    html += "<tr><td>" + tr("Size") + "</td><td>" + QString("%1").arg(f->fileInfo.size()) + "</td></tr>";
    html += "</table></body></html>";

    ui->textBrowser->setHtml(html);

    return true;
}
