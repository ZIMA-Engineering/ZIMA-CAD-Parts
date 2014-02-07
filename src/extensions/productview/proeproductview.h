#ifndef PROEPRODUCTVIEW_H
#define PROEPRODUCTVIEW_H

#include "abstractproductview.h"
#include "ui_proeproductview.h"

namespace Ui {
class ProEProductView;
}

class ProEProductView : public AbstractProductView
{
	Q_OBJECT
public:
	explicit ProEProductView(QWidget *parent = 0);
	~ProEProductView();

	QString title();
	QList<File::FileTypes> canHandle();
	bool handle(File *f);

signals:

public slots:

private:
	Ui::ProEProductView *ui;

};

#endif // PROEPRODUCTVIEW_H