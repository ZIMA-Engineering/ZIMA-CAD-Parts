/*
  ZIMA-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
