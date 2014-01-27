/*
  ZIMA-CAD-Parts
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

#ifndef PRODUCTVIEWSETTINGS_H
#define PRODUCTVIEWSETTINGS_H

#include "../../zima-cad-parts.h"

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

#endif // PRODUCTVIEWSETTINGS_H
