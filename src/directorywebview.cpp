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

#include "directorywebview.h"
#include "webdownloaderdialog.h"
#include "webauthenticationdialog.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QRegExp>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QWebEngineProfile>

#include "settings.h"
#include "zima-cad-parts.h"


DirectoryWebView::DirectoryWebView(QWidget *parent) :
    QWebEngineView(parent)
{
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChange(QUrl)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));

    connect(page(), SIGNAL(authenticationRequired(QUrl,QAuthenticator*)),
            this, SLOT(authenticate(QUrl,QAuthenticator*)));

    loadAboutPage();
}

void DirectoryWebView::setRootPath(QString path)
{
    m_rootPath = path;
}

void DirectoryWebView::loadAboutPage()
{
    QString url = ":/data/zima-cad-parts%1.html";
    QString localized = url.arg("_" + Settings::get()->getCurrentLanguageCode());
    QString filename = (QFile::exists(localized) ? localized : url.arg("") );

    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    QTextStream stream(&f);

    setHtml( stream.readAll().replace("%VERSION%", VERSION) );
}

DirectoryWebView* DirectoryWebView::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type)

    QDialog *popup = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout;

    DirectoryWebView *webview = new DirectoryWebView(this);
    layout->addWidget(webview);

    popup->setLayout(layout);
    popup->setWindowTitle(tr("ZIMA-CAD-Parts Technical Specifications"));
    popup->setWindowFlags(popup->windowFlags() | Qt::WindowMinMaxButtonsHint);
    popup->show();
    return webview;
}

void DirectoryWebView::urlChange(const QUrl &url)
{
    if(this->url() == url)
        return;

    if(url.scheme() == "about" || url.scheme() == "ZIMA-CAD-Parts")
        loadAboutPage();
}

void DirectoryWebView::pageLoaded(bool ok)
{
    Q_UNUSED(ok);

    if(url().scheme() != "file")
        return;

    // TODO:
    //   This does not really work, because m_rootPath is never set!
    //   It should be set to the current data source's root.
    page()->runJavaScript(QString(R"(
		window.ZCP = {rootPath: "%1"};
	)").arg(m_rootPath));

    page()->runJavaScript(R"(
		(function () {
		var elements = document.querySelectorAll('* [href], * [src]');
		for (var i = 0; i < elements.length; i++) {
			['href', 'src'].forEach(function (attr) {
				var v = elements[i].getAttribute(attr);

				if (v === null || !v.startsWith('/'))
					return;

				elements[i].setAttribute(attr, ZCP.rootPath + '/' + v);
			});
		}
		})()
	)");
}


void DirectoryWebView::authenticate(const QUrl &requestUrl, QAuthenticator *authenticator)
{
    Q_UNUSED(requestUrl)

    WebAuthenticationDialog dlg(authenticator);

    if (dlg.exec() != QDialog::Accepted)
        return;

    dlg.authenticate();
}
