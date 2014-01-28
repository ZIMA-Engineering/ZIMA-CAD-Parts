#include "pdfproductview.h"
#include "ui_pdfproductview.h"

#include <poppler/qt4/poppler-qt4.h>


PDFProductView::PDFProductView(QWidget *parent) :
	AbstractProductView(parent),
	ui(new Ui::PDFProductView)
{
	ui->setupUi(this);
}

PDFProductView::~PDFProductView()
{
	delete ui;
}

QString PDFProductView::title()
{
	return tr("PDF Document");
}

QList<File::FileTypes> PDFProductView::canHandle()
{
	return QList<File::FileTypes>() << File::PDF;
}

bool PDFProductView::handle(File *f)
{
	Poppler::Document* document = Poppler::Document::load(f->cachePath);
	if (!document || document->isLocked())
	{
		ui->imageLabel->setText(tr("Unable to load: %1").arg(f->cachePath));
		delete document;
		return false;
	}

	int pageNumber = 0;
	Poppler::Page* pdfPage = document->page(pageNumber);  // Document starts at page 0
	if (pdfPage == 0) {
		ui->imageLabel->setText(tr("Unable to load page %1 from %2").arg(pageNumber).arg(f->cachePath));
		delete document;
		return false;
	}

	QImage image = pdfPage->renderToImage();
	if (image.isNull()) {
		ui->imageLabel->setText(tr("Unable to load page %1 from %2").arg(pageNumber).arg(f->cachePath));
		delete document;
		return false;
	}

	ui->imageLabel->setPixmap(QPixmap::fromImage(image));

	delete pdfPage;
	delete document;

	return true;
}
