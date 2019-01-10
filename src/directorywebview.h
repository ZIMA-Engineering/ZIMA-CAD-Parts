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

#ifndef DIRECTORYWEBVIEW_H
#define DIRECTORYWEBVIEW_H

#include <QWebEngineView>

class WebDownloaderDialog;

class DirectoryWebView : public QWebEngineView
{
	Q_OBJECT
public:
	explicit DirectoryWebView(QWidget *parent = 0);
	void setRootPath(QString path);

signals:

public slots:
	void loadAboutPage();

protected:
	DirectoryWebView *createWindow(QWebEnginePage::WebWindowType type);

private slots:
	void pageLoaded(bool ok);
	void urlChange(const QUrl &url);
	void authenticate(const QUrl &requestUrl, QAuthenticator *authenticator);

private:
	QString m_rootPath;
};

#endif // DIRECTORYWEBVIEW_H
