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
#include <QPushButton>

#include "addeditdatasource.h"
#include "ui_addeditdatasource.h"

AddEditDataSource::AddEditDataSource(const DataSourceList &names, DataSource *dataSource, Actions action, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AddEditDataSource)
{
	ui->setupUi(this);
	m_action = action;

	foreach (DataSource *i, names)
	m_names.append(i->name);

	if (dataSource)
	{
		ui->labelLineEdit->setText(dataSource->name);
		ui->pathLineEdit->setText(dataSource->rootPath);
	}
	else
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	connect(ui->fileDialogButton, SIGNAL(clicked()),
	        this, SLOT(openFileDialog()));
	connect(ui->labelLineEdit, SIGNAL(textEdited(QString)),
	        this, SLOT(labelLineEdit_textEdited(QString)));
	connect(ui->pathLineEdit, SIGNAL(textEdited(QString)),
	        this, SLOT(pathLineEdit_textEdited(QString)));

	if( action == EDIT )
	{
		m_originalName = dataSource->name;
		setWindowTitle(tr("Edit data source"));
	}

	checkEnable();
}

AddEditDataSource::~AddEditDataSource()
{
	delete ui;
}

DataSource* AddEditDataSource::dataSource()
{
	return new DataSource(ui->labelLineEdit->text(), ui->pathLineEdit->text());
}

void AddEditDataSource::openFileDialog()
{
	QDir userEntered(ui->pathLineEdit->text());
	QString initDir;
	if (userEntered.exists())
		initDir = userEntered.absolutePath();
	else
		initDir = QDir::homePath();

	QString ret = QFileDialog::getExistingDirectory(this, tr("Select directory"), initDir);
	if (ret.isEmpty())
		return;
	ui->pathLineEdit->setText(ret);
	checkEnable();
}

void AddEditDataSource::labelLineEdit_textEdited(const QString &text)
{
	Q_UNUSED(text);
	checkEnable();
}

void AddEditDataSource::pathLineEdit_textEdited(const QString &text)
{
	Q_UNUSED(text);
	checkEnable();
}

void AddEditDataSource::checkEnable()
{
	// do not allow duplicated names for datasources
	QString label = ui->labelLineEdit->text().trimmed();
	QString errmsg;

	if (label.isEmpty())
	{
		errmsg = tr("Label/name of the data source cannot be empty");
	}
	else if (m_names.contains(label))
	{
		errmsg = tr("Label must be unique");
		if (m_action == EDIT && m_originalName == label)
		{
			errmsg = "";
		}
	}

	// directory existence
	if (ui->pathLineEdit->text().trimmed().isEmpty())
		errmsg += "\n" + tr("Enter the directory path");
	else
	{
		QDir d(ui->pathLineEdit->text());
		if (!d.exists())
			errmsg += "\n" + tr("Directory must exist");
	}

	ui->statusLabel->setText(errmsg.trimmed());
	ui->statusLabel->setVisible(!errmsg.isEmpty());
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(errmsg.isEmpty());
}
