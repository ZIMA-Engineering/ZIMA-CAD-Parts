#ifdef HAVE_POPPLER
#ifndef PDFPRODUCTVIEW_H
#define PDFPRODUCTVIEW_H

#include <QWidget>

#include "abstractproductview.h"

namespace Ui {
class PDFProductView;
}


/**
 * @brief The PDFProductView class is a product view for PDF files
 * @see AbstractProductView
 */
class PDFProductView : public AbstractProductView
{
    Q_OBJECT

public:
    explicit PDFProductView(QWidget *parent = 0);
    ~PDFProductView();

    QString title();
    QList<File::FileTypes> canHandle();
    bool handle(FileMetadata *f);

private:
    Ui::PDFProductView *ui;
};

#endif // PDFPRODUCTVIEW_H

#endif
