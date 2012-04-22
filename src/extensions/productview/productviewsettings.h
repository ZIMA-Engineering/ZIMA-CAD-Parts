#ifndef PRODUCTVIEWSETTINGS_H
#define PRODUCTVIEWSETTINGS_H

#include "../../zima-parts.h"

#ifdef INCLUDE_PRODUCT_VIEW

#include <QWidget>
#include <QSettings>

#define PRODUCT_VIEW_DEFAULT_PATH "C:\\Program Files\\ProductViewExpress"

namespace Ui {
class ProductViewSettings;
}

class ProductViewSettings : public QWidget
{
	Q_OBJECT
	
public:
	explicit ProductViewSettings(QSettings *settings, QWidget *parent = 0);
	~ProductViewSettings();
	void saveSettings();
	
private:
	Ui::ProductViewSettings *ui;
	QSettings *settings;

private slots:
	void setPathDialog();
};

#endif // INCLUDE_PRODUCT_VIEW
#endif // PRODUCTVIEWSETTINGS_H
