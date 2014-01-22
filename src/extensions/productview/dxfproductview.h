#ifndef DXFPRODUCTVIEW_H
#define DXFPRODUCTVIEW_H

#include <QWidget>

#include "abstractproductview.h"

namespace Ui {
class DxfProductView;
}

class DXFInterface;


class DxfProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit DxfProductView(QWidget *parent = 0);
    ~DxfProductView();

    QString title();
    QList<File::FileTypes> canHandle();
    bool handle(File *f);

private:
    Ui::DxfProductView *ui;
    DXFInterface *dxf;
};

#endif // DXFPRODUCTVIEW_H
