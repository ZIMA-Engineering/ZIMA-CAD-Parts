#include "dxfproductview.h"
#include "ui_dxfproductview.h"

#include <dxfinterface.h>
#include <libdxfrw.h>
#include <dxfsceneview.h>

DxfProductView::DxfProductView(QWidget *parent) :
	AbstractProductView(parent),
	ui(new Ui::DxfProductView),
	dxf(0)
{
	ui->setupUi(this);
}

DxfProductView::~DxfProductView()
{
	delete ui;
}

QString DxfProductView::title()
{
	return tr("DXF part");
}

QList<File::FileType> DxfProductView::canHandle()
{
	return QList<File::FileType>() << File::DXF;
}

bool DxfProductView::handle(File *f)
{
	if (dxf)
	{
		delete dxf;
		dxf = 0;
	}

    dxf = new DXFInterface(f->fileInfo.absoluteFilePath());

	ui->view->setScene(dxf->scene());
	ui->view->fitInView(dxf->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);

	return true;
}
