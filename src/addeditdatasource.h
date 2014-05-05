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

#ifndef ADDEDITDATASOURCE_H
#define ADDEDITDATASOURCE_H

#include <QDialog>
#include <QList>
#include "basedatasource.h"
#include "ftpdatasource.h"
#include "localdatasource.h"

namespace Ui {
class AddEditDataSource;
}

class AddEditDataSource : public QDialog
{
	Q_OBJECT

public:
	enum Actions {
		ADD=0,
		EDIT
	};

	explicit AddEditDataSource(BaseDataSource *dataSource, Actions action, QWidget *parent = 0);
	~AddEditDataSource();

	BaseDataSource *dataSource();

private:
	void refill();

	Ui::AddEditDataSource *ui;
    DataSourceList dataSources;
	BaseDataSource *lastDataSource;
private slots:
	void openFileDialog();
	void dataSourceTypeChanged(int index);
	void labelChangedByUser();
};

#endif // ADDEDITDATASOURCE_H
