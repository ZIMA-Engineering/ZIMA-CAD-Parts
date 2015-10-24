#include "imageproductview.h"
#include "ui_imageproductview.h"

#include <QPixmap>


ImageProductView::ImageProductView(QWidget *parent) :
    AbstractProductView(parent),
    ui(new Ui::ImageProductView)
{
    ui->setupUi(this);
}

ImageProductView::~ImageProductView()
{
    delete ui;
}

QString ImageProductView::title()
{
    return tr("Image File");
}

FileTypeList ImageProductView::canHandle()
{
    return FileTypeList() << FileType::FILE_IMAGE;
}

bool ImageProductView::handle(FileMetadata *f)
{
    QPixmap p(f->fileInfo.absoluteFilePath());
    if (!p.isNull())
    {
        ui->label->setPixmap(p);
        return true;
    }
    else
    {
        ui->label->setText(tr("Cannot load image: <a href=\"file://%1\">%2</a>")
                           .arg(f->fileInfo.absoluteFilePath())
                           .arg(f->fileInfo.fileName())
                           );
        return false;
    }
}
