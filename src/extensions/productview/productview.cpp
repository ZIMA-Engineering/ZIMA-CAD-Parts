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
#include "../../settings.h"

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
#ifdef HAVE_POPPLER
    addProviders<PDFProductView>();
#endif
	//failbackProvider = new FailbackProductView(this);
	//failbackProvider->hide();
	failbackProvider = 0;
}

ProductView::~ProductView()
{
	saveSettings();

	delete ui;

	QHashIterator<File::FileType, AbstractProductView*> i(providers);
	while (i.hasNext())
	{
		i.next();
		i.value()->deleteLater();
	}

	providers.clear();
}

bool ProductView::canHandle(File::FileType t)
{
	return providers.contains(t);
}

void ProductView::hideEvent(QHideEvent * e)
{
	saveSettings();
	QDialog::hideEvent(e);
}

void ProductView::showEvent(QShowEvent *e)
{
	restoreGeometry(Settings::get()->ExtensionsProductViewGeometry);
	QPoint pt = Settings::get()->ExtensionsProductViewPosition;
	if (!pt.isNull())
		move(pt);

	QDialog::showEvent(e);
}

void ProductView::saveSettings()
{
	Settings::get()->ExtensionsProductViewGeometry = saveGeometry();
	Settings::get()->ExtensionsProductViewPosition = pos();
}

void ProductView::setFile(File* f)
{
	if (currentProvider)
	{
		currentProvider->hide();
		ui->verticalLayout->removeWidget(currentProvider);
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

    setWindowTitle(f->fileInfo.baseName() + " " + currentProvider->title());

	//qDebug() << "PTH" << f->path << f->targetPath;
	//ui->statusLabel->setText(tr("Displaying: %1").arg(currentProvider->title()));
	currentProvider->handle(f);
	//ui->verticalLayout->insertWidget(1, currentProvider);
	ui->verticalLayout->addWidget(currentProvider);
	currentProvider->show();
}
