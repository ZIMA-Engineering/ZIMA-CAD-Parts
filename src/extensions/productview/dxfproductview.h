#ifndef DXFPRODUCTVIEW_H
#define DXFPRODUCTVIEW_H

#include <QWidget>

#include "abstractproductview.h"

namespace Ui {
class DxfProductView;
}

class DXFInterface;


/**
 * @brief The DxfProductView class is a product view for DXF files
 * @see AbstractProductView
 */
class DxfProductView : public AbstractProductView
{
	Q_OBJECT

public:
	explicit DxfProductView(QWidget *parent = 0);
	~DxfProductView();

	QString title();
    FileTypeList canHandle();
    bool handle(FileMetadata *f);

private:
	Ui::DxfProductView *ui;
	DXFInterface *dxf;
};

#endif // DXFPRODUCTVIEW_H
