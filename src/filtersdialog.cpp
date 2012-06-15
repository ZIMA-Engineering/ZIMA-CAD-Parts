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

#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

#include "filtersdialog.h"
#include "ui_filtersdialog.h"
#include "item.h"

FiltersDialog::FiltersDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::FiltersDialog)
{
	ui->setupUi(this);

	QGridLayout *grid = new QGridLayout;

	int cnt = MainWindow::filterGroups.count();
	int row = 0, col = 0;

	for(int i = 0; i < cnt; i++)
	{
		GroupCheckBox *groupCheckBoxes = new GroupCheckBox;

		QGroupBox *group = new QGroupBox(this);
		group->setTitle(MainWindow::filterGroups[i].label);
		group->setCheckable(true);
		group->setChecked(MainWindow::filterGroups[i].enabled);

		groupCheckBoxes->groupBox = group;

		QVBoxLayout *groupLayout = new QVBoxLayout;

		int filterCnt = MainWindow::filterGroups[i].filters.count();

		for(int j = 0; j < filterCnt; j++)
		{
			QCheckBox *chckBox = new QCheckBox(File::getLabelForFileType(MainWindow::filterGroups[i].filters[j].type), this);
			chckBox->setChecked(MainWindow::filterGroups[i].filters[j].enabled);

			groupCheckBoxes->checkBoxes << chckBox;

			groupLayout->addWidget(chckBox);
		}

		groupLayout->addStretch(100);

		groups << groupCheckBoxes;

		group->setLayout(groupLayout);
		grid->addWidget(group, row, col++);

		if(col >= 4)
		{
			row++;
			col = 0;
		}
	}

	ui->verticalLayout->insertLayout(0, grid);
}

FiltersDialog::~FiltersDialog()
{
	delete ui;

	qDeleteAll(groups);
}

void FiltersDialog::accept()
{
	int cnt = MainWindow::filterGroups.count();

	for(int i = 0; i < cnt; i++)
	{
		MainWindow::filterGroups[i].enabled = groups[i]->groupBox->isChecked();

		int filterCnt = MainWindow::filterGroups[i].filters.count();

		for(int j = 0; j < filterCnt; j++)
			MainWindow::filterGroups[i].filters[j].enabled = groups[i]->checkBoxes[j]->isChecked();
	}

	QDialog::accept();
}

