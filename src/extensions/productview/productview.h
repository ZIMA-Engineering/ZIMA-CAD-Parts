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

#ifndef PRODUCTVIEW_H
#define PRODUCTVIEW_H

#include "../../zima-cad-parts.h"

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
