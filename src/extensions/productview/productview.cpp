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

#include "productview.h"


#include "ui_productview.h"
#include "proeproductview.h"
#include "dxfproductview.h"
#include "pdfproductview.h"


ProductView::ProductView(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ProductView()),
	currentProvider(0)
{
	ui->setupUi(this);
//	ui->statusLabel->setText(tr("Double click any part."));
	setWindowFlags(Qt::Window);

	addProviders<ProEProductView>();
	addProviders<DxfProductView>();
	// disabled 20140206 by Vlad's request:
	// only "supported" files should be displayed. For rest of files this dialog should be closed
	// addProviders<PDFProductView>();
	//failbackProvider = new FailbackProductView(this);
	//failbackProvider->hide();
	failbackProvider = 0;
}

ProductView::~ProductView()
{
	saveSettings();

	delete ui;

	QHashIterator<File::FileTypes, AbstractProductView*> i(providers);
	while (i.hasNext())
	{
		i.next();
		i.value()->deleteLater();
	}

	providers.clear();
}

bool ProductView::canHandle()
{
	return currentProvider != 0;
}

void ProductView::hideEvent(QHideEvent * e)
{
	saveSettings();
	QDialog::hideEvent(e);
}

void ProductView::showEvent(QShowEvent *e)
{
	QSettings s;
	restoreGeometry(s.value("Extensions/ProductView/geometry").toByteArray());
	QPoint pt = s.value("Extensions/ProductView/position").toPoint();
	if (!pt.isNull())
		move(pt);

	QDialog::showEvent(e);
}

void ProductView::saveSettings()
{
	QSettings s;
	s.setValue("Extensions/ProductView/geometry", saveGeometry());
	s.setValue("Extensions/ProductView/position", pos());
}


bool ProductView::expectFile(File *f)
{
	if (expectedFile != f)
	{
		expectedFile = f;
		return true;
	}
	return false;
//	ui->statusLabel->setText(tr("Waiting for part to download..."));
}

void ProductView::fileDownloaded(File *f)
{
	if (currentProvider)
	{
		currentProvider->hide();
		ui->verticalLayout->removeWidget(currentProvider);
	}

	if (f != expectedFile)
	{
//		ui->statusLabel->setText(tr("Double click any part."));
		return;
	}

	if (!providers.contains(f->type))
	{
		//currentProvider = failbackProvider;
		currentProvider = 0;
		return;
	}
	else
	{
		currentProvider = providers.value(f->type);
	}

	setWindowTitle(f->baseName() + " " + currentProvider->title());

	//qDebug() << "PTH" << f->path << f->targetPath;
	//ui->statusLabel->setText(tr("Displaying: %1").arg(currentProvider->title()));
	currentProvider->handle(f);
	//ui->verticalLayout->insertWidget(1, currentProvider);
	ui->verticalLayout->addWidget(currentProvider);
	currentProvider->show();
}
