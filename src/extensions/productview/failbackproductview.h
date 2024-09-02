#ifndef FAILBACKPRODUCTVIEW_H
#define FAILBACKPRODUCTVIEW_H

#include "abstractproductview.h"


namespace Ui {
class FailbackProductView;
}

/**
 * @brief The FailbackProductView class is a product view for unhandled files
 * @see AbstractProductView
 */
class FailbackProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit FailbackProductView(QWidget *parent = 0);
    ~FailbackProductView();

    QString title();
    FileTypeList canHandle();
    bool handle(FileMetadata *f);

private:
    Ui::FailbackProductView *ui;
};

#endif // FAILBACKPRODUCTVIEW_H
