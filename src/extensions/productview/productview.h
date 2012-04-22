#ifndef PRODUCTVIEW_H
#define PRODUCTVIEW_H

#include "../../zima-parts.h"

#ifdef INCLUDE_PRODUCT_VIEW

#include <QWidget>
#include <QSettings>

#include "../../item.h"

namespace Ui {
class ProductView;
}

class ProductView : public QWidget
{
	Q_OBJECT
	
public:
	explicit ProductView(QSettings *settings, QWidget *parent = 0);
	~ProductView();
	bool isExtensionEnabled() const;

public slots:
	void expectFile(File* f);
	void fileDownloaded(File* f);
	
private:
	Ui::ProductView *ui;
	QSettings *settings;
	File *expectedFile;
};

#endif // INCLUDE_PRODUCT_VIEW

#endif // PRODUCTVIEW_H
