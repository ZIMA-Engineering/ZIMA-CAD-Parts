#ifndef PDFPRODUCTVIEW_H
#define PDFPRODUCTVIEW_H

#include <QWidget>

#include "abstractproductview.h"

namespace Ui {
class PDFProductView;
}

class PDFProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit PDFProductView(QWidget *parent = 0);
    ~PDFProductView();

    QString title();
    QList<File::FileTypes> canHandle();
    bool handle(File *f);

private:
    Ui::PDFProductView *ui;
};

#endif // PDFPRODUCTVIEW_H
