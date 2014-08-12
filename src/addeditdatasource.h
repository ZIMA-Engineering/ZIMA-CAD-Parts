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
#include "settings.h"

class DataSources;

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

    explicit AddEditDataSource(const DataSourceList &names, DataSource *dataSource, Actions action, QWidget *parent = 0);
	~AddEditDataSource();

    DataSource *dataSource();

private:
	Ui::AddEditDataSource *ui;
    DataSource *m_ds;
    QStringList m_names;
    QString m_originalName;
    Actions m_action;

    //! User data validations
    void checkEnable();

private slots:
	void openFileDialog();
    void labelLineEdit_textEdited(const QString &text);
    void pathLineEdit_textEdited(const QString &text);
};

#endif // ADDEDITDATASOURCE_H
