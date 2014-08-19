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
#include "settings.h"


FiltersDialog::FiltersDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FiltersDialog)
{
	ui->setupUi(this);

	int cnt = Settings::get()->FilterGroups.count();
	for(int i = 0; i < cnt; i++)
	{
		ui->treeWidget->addTopLevelItem(Settings::get()->FilterGroups[i].widget());
		ui->listWidget->addItem(Settings::get()->FilterGroups[i].label);
	}

	ui->treeWidget->expandAll();
	ui->treeWidget->setFocus();

	connect(ui->listWidget, SIGNAL(currentRowChanged(int)),
	        this, SLOT(listWidget_currentRowChanged(int)));
	connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	        this, SLOT(treeWidget_currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

FiltersDialog::~FiltersDialog()
{
	delete ui;
}

void FiltersDialog::accept()
{
	int cnt = Settings::get()->FilterGroups.count();

	for(int i = 0; i < cnt; i++)
		Settings::get()->FilterGroups[i].apply();

    Settings::get()->recalculateFilters();
	QDialog::accept();
}

void FiltersDialog::listWidget_currentRowChanged(int row)
{
	ui->listWidget->blockSignals(true);
	ui->treeWidget->setCurrentItem(Settings::get()->FilterGroups[row].currentItem());
	ui->listWidget->blockSignals(false);
}

void FiltersDialog::treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (!current)
		return;

	ui->listWidget->blockSignals(true);
	QTreeWidgetItem *parent = current->parent();
	if (!parent)
		parent = current;

	int ix = ui->treeWidget->indexOfTopLevelItem(parent);
	ui->listWidget->setCurrentRow(ix);
	ui->listWidget->blockSignals(false);
}
