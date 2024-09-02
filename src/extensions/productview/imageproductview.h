#ifndef IMAGEPRODUCTVIEW_H
#define IMAGEPRODUCTVIEW_H

#include "abstractproductview.h"

namespace Ui {
class ImageProductView;
}

class ImageProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit ImageProductView(QWidget *parent = 0);
    ~ImageProductView();

    QString title();
    FileTypeList canHandle();
    bool handle(FileMetadata *f);

private:
    Ui::ImageProductView *ui;
};

#endif // IMAGEPRODUCTVIEW_H
