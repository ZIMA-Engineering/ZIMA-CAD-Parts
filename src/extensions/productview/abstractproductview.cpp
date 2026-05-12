#include "abstractproductview.h"

AbstractProductView::AbstractProductView(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(windowFlags()
                   | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint);
}
