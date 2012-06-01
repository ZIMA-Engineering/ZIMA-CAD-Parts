/*
  ZIMA-Parts
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

#ifdef INCLUDE_PRODUCT_VIEW

#include "ui_productview.h"

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDebug>

#include "productviewsettings.h"

ProductView::ProductView(QSettings *settings, QWidget *parent) :
        QWidget(parent),
	ui(new Ui::ProductView),
	settings(settings)
{
	ui->setupUi(this);

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavaEnabled, true);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);

	ui->appletWebView->setHtml(tr("Double click any PRO/E part."));
}

ProductView::~ProductView()
{
	delete ui;
}

bool ProductView::isExtensionEnabled() const
{
	return settings->value("Extensions/ProductView/Enabled", false).toBool();
}

void ProductView::expectFile(File *f)
{
	expectedFile = f;

	ui->appletWebView->setHtml(tr("Waiting for part to download..."));
}

void ProductView::fileDownloaded(File *f)
{
	if(f == expectedFile)
	{
		QFile pv(":/data/extensions/productview/productview.html");
		pv.open(QIODevice::ReadOnly);
		QTextStream stream(&pv);
		QString html = stream.readAll();

		html.replace("%VERSION%", VERSION);
		html.replace("%FILE_NAME%", f->name);
		html.replace("%FILE_PATH%", f->targetPath);
		html.replace("%PRODUCTVIEW_PATH%", settings->value("Extensions/ProductView/Path", PRODUCT_VIEW_DEFAULT_PATH).toString());

		QTemporaryFile tmp(QDir::tempPath() + "/zima-parts_XXXXXX_" + f->name + ".html");

		if(tmp.open())
		{
			QTextStream out(&tmp);
			out << html;
			out.flush();

			// It doesn't work with double slash in windows
			ui->appletWebView->setUrl(QUrl(QString("file:/%1").arg(tmp.fileName())));

			//tmp.setAutoRemove(false);
			tmp.close();
		} else {
			ui->appletWebView->setHtml("Sorry, cannot create temporary file.");
		}
	}
}

#endif // INCLUDE_PRODUCT_VIEW
