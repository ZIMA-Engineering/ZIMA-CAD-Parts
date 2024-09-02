#ifndef PROEPRODUCTVIEW_H
#define PROEPRODUCTVIEW_H

#include "abstractproductview.h"
#include "ui_proeproductview.h"

namespace Ui {
class ProEProductView;
}

/**
 * @brief The ProEProductView class is a product view for PRO/E files
 * @see AbstractProductView
 */
class ProEProductView : public AbstractProductView
{
    Q_OBJECT
public:
    explicit ProEProductView(QWidget *parent = 0);
    ~ProEProductView();

    QString title();
    FileTypeList canHandle();
    bool handle(FileMetadata *f);

signals:

public slots:

private:
    Ui::ProEProductView *ui;

};

#endif // PROEPRODUCTVIEW_H
