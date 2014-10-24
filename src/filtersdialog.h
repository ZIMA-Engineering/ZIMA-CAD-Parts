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

#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include <QDialog>
#include <QList>
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QTreeWidgetItem>


namespace Ui {
class FiltersDialog;
}

class FiltersDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FiltersDialog(QWidget *parent = 0);
	~FiltersDialog();
	void accept();

private:
	Ui::FiltersDialog *ui;

private slots:
	void listWidget_currentRowChanged(int row);
	void treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*);
};

#endif // FILTERSDIALOG_H
