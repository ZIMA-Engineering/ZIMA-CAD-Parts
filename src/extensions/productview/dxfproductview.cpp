#include "dxfproductview.h"
#include "ui_dxfproductview.h"

#include <dxfinterface.h>
#include <libdxfrw.h>
#include <dxfsceneview.h>
#include <QPushButton>

DxfProductView::DxfProductView(QWidget *parent) :
    AbstractProductView(parent),
    ui(new Ui::DxfProductView),
    dxf(0)
{
    ui->setupUi(this);

    connect(ui->fitButton, &QPushButton::clicked,
            ui->view, &DXFSceneView::fitAll);
}

DxfProductView::~DxfProductView()
{
    delete ui;
}

QString DxfProductView::title()
{
    return tr("DXF part");
}

FileTypeList DxfProductView::canHandle()
{
    return FileTypeList() << FileType::DXF;
}

bool DxfProductView::handle(FileMetadata *f)
{
    if (dxf)
    {
        delete dxf;
        dxf = 0;
    }

    dxf = new DXFInterface(f->fileInfo.absoluteFilePath());

    ui->view->setScene(dxf->scene());
    ui->view->fitAll();

    return true;
}
