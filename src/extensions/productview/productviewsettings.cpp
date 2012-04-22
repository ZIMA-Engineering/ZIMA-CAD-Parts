#include "productviewsettings.h"

#ifdef INCLUDE_PRODUCT_VIEW

#include "ui_productviewsettings.h"

#include <QFileDialog>

ProductViewSettings::ProductViewSettings(QSettings *settings, QWidget *parent) :
        QWidget(parent),
	ui(new Ui::ProductViewSettings),
	settings(settings)
{
	ui->setupUi(this);

	connect(ui->pathButton, SIGNAL(clicked()), this, SLOT(setPathDialog()));

	ui->enabledCheckBox->setChecked(settings->value("Extensions/ProductView/Enabled", false).toBool());
	ui->pathLineEdit->setText(settings->value("Extensions/ProductView/Path", PRODUCT_VIEW_DEFAULT_PATH).toString());
}

ProductViewSettings::~ProductViewSettings()
{
	delete ui;
}

void ProductViewSettings::setPathDialog()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("ZIMA-Parts - set ProductView path"), ui->pathLineEdit->text());
	if (!str.isEmpty())
		ui->pathLineEdit->setText(str);
}

void ProductViewSettings::saveSettings()
{
	settings->setValue("Extensions/ProductView/Enabled", ui->enabledCheckBox->isChecked());
	settings->setValue("Extensions/ProductView/Path", ui->pathLineEdit->text());
}

#endif // INCLUDE_PRODUCT_VIEW
