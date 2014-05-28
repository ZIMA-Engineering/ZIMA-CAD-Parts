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

#include "techspecswebview.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QRegExp>
#include <QDebug>

#include "datatransfer.h"
#include "downloadmodel.h"
#include "settings.h"
#include "zima-cad-parts.h"


TechSpecsWebView::TechSpecsWebView(QWidget *parent) :
	QWebView(parent)
{
    m_downloadModel = new DownloadModel(this, this);

	loadAboutPage();

	connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChange(QUrl)));
	connect(this, SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));

	page()->setForwardUnsupportedContent(true);
	connect(page(), SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadFile(QNetworkReply*)));
}

void TechSpecsWebView::setRootPath(QString path)
{
	m_rootPath = path;
}

void TechSpecsWebView::setDownloadDirectory(QString path)
{
	m_dlDir = path;
}

void TechSpecsWebView::stopDownload()
{
    QList<File*> tmp = m_downloadModel->files();

	foreach(File *f, tmp)
	f->transfer->cancel();
}

void TechSpecsWebView::resumeDownload()
{
    QList<File*> tmp = m_downloadModel->files();

	foreach(File *f, tmp)
	downloadFile(page()->networkAccessManager()->get(QNetworkRequest(f->path)), f);

}

void TechSpecsWebView::clearQueue()
{
    QList<File*> tmp = m_downloadModel->files();

	foreach(File *f, tmp)
	delete f->transfer;
}

void TechSpecsWebView::loadAboutPage()
{
	QString url = ":/data/zima-cad-parts%1.html";
    QString localized = url.arg("_" + Settings::get()->getCurrentLanguageCode());
	QString filename = (QFile::exists(localized) ? localized : url.arg("") );

	QFile f(filename);
	f.open(QIODevice::ReadOnly);
	QTextStream stream(&f);

	setHtml( stream.readAll().replace("%VERSION%", VERSION) );
}

TechSpecsWebView* TechSpecsWebView::createWindow(QWebPage::WebWindowType type)
{
	QDialog *popup = new QDialog(this);
	QVBoxLayout *layout = new QVBoxLayout;

	TechSpecsWebView *webview = new TechSpecsWebView(this);
	layout->addWidget(webview);

	popup->setLayout(layout);
	popup->setWindowTitle(tr("ZIMA-CAD-Parts Technical Specifications"));

	if(type == QWebPage::WebModalDialog)
		popup->setModal(true);

	popup->setWindowFlags(popup->windowFlags() | Qt::WindowMinMaxButtonsHint);
	popup->show();
	return webview;
}

void TechSpecsWebView::urlChange(const QUrl &url)
{
	if(this->url() == url)
		return;

	if(url.scheme() == "about" || url.scheme() == "ZIMA-CAD-Parts")
		loadAboutPage();
}

void TechSpecsWebView::pageLoaded(bool ok)
{
	Q_UNUSED(ok);

	if(url().scheme() != "file")
		return;

	QWebFrame *frame = page()->mainFrame();

	QWebElementCollection collection = frame->findAllElements("* [href], * [src]");

	foreach(QWebElement el, collection)
	{
		foreach(QString attr, el.attributeNames())
		{
			QString val = el.attribute(attr);

			if(!val.startsWith('/'))
				continue;

			val.insert(0, m_rootPath);

			el.setAttribute(attr, val);
		}
	}
}

void TechSpecsWebView::downloadFile(QNetworkReply *reply, File *f)
{
	bool isNew = !f;

	if(isNew)
	{
		QString fileName = reply->url().path().split('/').last();

		if(reply->hasRawHeader("Content-Disposition"))
		{
			QStringList patterns;
			patterns << "filename=\"(.+)\"" << "filename=([^$]+)";

			QRegExp rx;

			foreach(QString pattern, patterns)
			{
				rx.setPattern(pattern);

				if(rx.indexIn(reply->rawHeader("Content-Disposition")) != -1)
				{
					fileName = rx.cap(1).replace('\\', '/').split('/').last();

					if(fileName.endsWith(';'))
						fileName.chop(1);

					break;
				}
			}
		}

		f = new File;
		f->parentItem = 0;
		f->name = fileName;
		f->path = reply->url().toString();
		f->targetPath = m_dlDir + "/" + fileName;
		f->transferHandler = DownloadModel::TechSpec;
		f->transfer = new DataTransfer(reply, f);
		f->transfer->setDeleteSrc(true);
		f->transfer->setDeleteDst(true);
	} else {
		f->bytesDone = 0;
		f->transfer->setSource(reply);
	}

	if(reply->hasRawHeader("Content-Length"))
		f->size = reply->rawHeader("Content-Length").toULongLong();

	if(f->transfer->initiate() && isNew)
        m_downloadModel->enqueue(f);
}
