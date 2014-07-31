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

#include <QDir>
#include <QFileDialog>
#include <QDebug>

#include "addeditdatasource.h"
#include "ui_addeditdatasource.h"

AddEditDataSource::AddEditDataSource(BaseDataSource *dataSource, Actions action, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AddEditDataSource)
{
	ui->setupUi(this);

	dataSources << dataSource;
	lastDataSource = dataSource;

	refill();

	connect(ui->fileDialogButton, SIGNAL(clicked()), this, SLOT(openFileDialog()));
    connect(ui->labelLineEdit, SIGNAL(textEdited(QString)), this, SLOT(labelChangedByUser()));

	if( action == EDIT )
		setWindowTitle(tr("Edit data source"));
}

AddEditDataSource::~AddEditDataSource()
{
	delete ui;

	foreach(BaseDataSource *bs, dataSources)
	if( bs != lastDataSource )
		delete bs;
}

BaseDataSource* AddEditDataSource::dataSource()
{
	lastDataSource->label = ui->labelLineEdit->text();

    LocalDataSource *s = static_cast<LocalDataSource*>(lastDataSource);
    s->localPath = ui->pathLineEdit->text();
	return lastDataSource;
}

void AddEditDataSource::openFileDialog()
{
	ui->pathLineEdit->setText( QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath()) );
}

void AddEditDataSource::refill()
{
	ui->labelLineEdit->setText(lastDataSource->label);

    LocalDataSource *s = static_cast<LocalDataSource*>(lastDataSource);

    if( !s->label.isEmpty() && s->localPath != s->label )
        labelChangedByUser();
    ui->pathLineEdit->setText( s->localPath );
}

#warning TODO/FIXME: simplify it
void AddEditDataSource::labelChangedByUser()
{
	disconnect(ui->pathLineEdit, SIGNAL(textChanged(QString)), ui->labelLineEdit, SLOT(setText(QString)));
}
