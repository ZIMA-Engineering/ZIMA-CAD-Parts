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
#include "browserpage.h"

#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QTextStream>
#include <QUrlQuery>

#include "metadata.h"
#include "settings.h"
#include "zima-cad-parts.h"

namespace {

QStringList imageExtensions()
{
    return QStringList() << "png" << "jpg" << "jpeg" << "gif";
}

QString findImageFile(const QString &path, const QString &baseName)
{
    foreach (const QString &extension, imageExtensions())
    {
        QString fileName = QString("%1/%2.%3").arg(path).arg(baseName).arg(extension);

        if (QFile::exists(fileName))
            return fileName;
    }

    return QString();
}

QString findEntryThumbnail(const QString &path, const QString &baseName)
{
    QString local = findImageFile(path, baseName);

    if (!local.isEmpty())
        return local;

    return findImageFile(path + "/" + THUMBNAILS_DIR, baseName);
}

QString findIncludedThumbnail(Metadata *metadata, const QString &baseName, QSet<QString> *visited)
{
    if (!metadata || visited->contains(metadata->path()))
        return QString();

    visited->insert(metadata->path());

    QString local = findEntryThumbnail(metadata->path(), baseName);

    if (!local.isEmpty())
        return local;

    foreach (Metadata *include, metadata->thumbnailIncludes())
    {
        QString included = findIncludedThumbnail(include, baseName, visited);

        if (!included.isEmpty())
            return included;
    }

    return QString();
}

} // namespace


DirectoryWebView::DirectoryWebView(QWidget *parent) :
    QWebEngineView(parent)
{
    BrowserPage *browserPage = new BrowserPage(this);
    connect(browserPage, SIGNAL(openDirectoryRequested(QString)),
            this, SIGNAL(openDirectoryRequested(QString)));
    setPage(browserPage);

    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChange(QUrl)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));

    loadAboutPage();
}

void DirectoryWebView::setRootPath(QString path)
{
    m_rootPath = path;
    m_autoIndexUrl.clear();
}

bool DirectoryWebView::isAutoIndexPage() const
{
    if (m_autoIndexUrl.isEmpty() || !url().isLocalFile())
        return false;

    return QDir::cleanPath(url().toLocalFile())
            == QDir::cleanPath(m_autoIndexUrl.toLocalFile());
}

void DirectoryWebView::loadAboutPage()
{
    m_autoIndexUrl.clear();

    QString url = ":/data/zima-cad-parts%1.html";
    QString localized = url.arg("_" + Settings::get()->getCurrentLanguageCode());
    QString filename = (QFile::exists(localized) ? localized : url.arg("") );

    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    QTextStream stream(&f);

    setHtml( stream.readAll().replace("%VERSION%", VERSION) );
}

void DirectoryWebView::loadAutoIndexPage(const QString &path)
{
    m_rootPath = path;
    m_autoIndexUrl = QUrl::fromLocalFile(path + "/");

    setHtml(autoIndexHtml(path), m_autoIndexUrl);
}

void DirectoryWebView::urlChange(const QUrl &url)
{
    if ((url.scheme() == "about" && url != QUrl("about:blank"))
            || url.scheme() == "ZIMA-CAD-Parts")
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

QString DirectoryWebView::autoIndexHtml(const QString &path) const
{
    QDir dir(path);
    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot,
                                           QDir::Name);
    QString tiles;

    foreach (const QFileInfo &child, dirs)
    {
        if (child.fileName() == METADATA_DIR)
            continue;

        tiles += autoIndexTileHtml(path, child);
    }

    if (tiles.isEmpty())
    {
        tiles = QString("<div class=\"empty\">%1</div>")
                .arg(tr("No subdirectories").toHtmlEscaped());
    }

    QString title = autoIndexTitle(path).toHtmlEscaped();
    QString subtitle = tr("Auto-generated directory index").toHtmlEscaped();

    return QString(R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>%1</title>
<style>
body {
    color: #222;
    font-family: Verdana, Arial, sans-serif;
    margin: 22px;
    background: #fff;
}
.header {
    align-items: center;
    border-bottom: 1px solid #d8d8d8;
    display: flex;
    gap: 18px;
    justify-content: space-between;
    margin-bottom: 24px;
    padding-bottom: 14px;
}
.title {
    font-size: 28px;
    font-weight: bold;
    margin: 0 0 4px;
}
.subtitle {
    color: #666;
    font-size: 13px;
}
.logo {
    max-height: 46px;
    max-width: 240px;
}
.grid {
    display: grid;
    gap: 18px;
    grid-template-columns: repeat(auto-fill, minmax(220px, 1fr));
}
.tile {
    background: #fafafa;
    border: 1px solid #d0d0d0;
    color: #111;
    display: block;
    min-height: 210px;
    padding: 14px;
    text-align: center;
    text-decoration: none;
}
.tile:hover {
    border-color: #999;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.16);
}
.name {
    font-size: 15px;
    font-weight: bold;
    line-height: 1.3;
    margin-bottom: 14px;
    word-break: break-word;
}
.thumbnail {
    max-height: 150px;
    max-width: 100%;
}
.placeholder,
.empty {
    align-items: center;
    border: 1px dashed #aaa;
    color: #666;
    display: flex;
    justify-content: center;
}
.placeholder {
    height: 150px;
}
.empty {
    min-height: 120px;
}
</style>
</head>
<body>
<div class="header">
    <div>
        <h1 class="title">%1</h1>
        <div class="subtitle">%2</div>
    </div>
    <img class="logo" src="qrc:/gfx/zima-engineering-logo.svg" alt="">
</div>
<div class="grid">
%3
</div>
</body>
</html>)").arg(title, subtitle, tiles);
}

QString DirectoryWebView::autoIndexTitle(const QString &path) const
{
    QString label = MetadataCache::get()->label(path);

    if (!label.isEmpty())
        return label;

    QString name = QFileInfo(path).fileName();

    if (!name.isEmpty())
        return name;

    return path;
}

QString DirectoryWebView::autoIndexTileHtml(const QString &rootPath, const QFileInfo &dir) const
{
    QString label = MetadataCache::get()->label(dir.absoluteFilePath());

    if (label.isEmpty())
        label = dir.fileName();

    QString imagePath = thumbnailPath(rootPath, dir);
    QString preview;

    if (imagePath.isEmpty())
    {
        preview = QString("<div class=\"placeholder\">%1</div>")
                  .arg(tr("Open directory").toHtmlEscaped());
    } else {
        preview = QString("<img class=\"thumbnail\" src=\"%1\" alt=\"\">")
                  .arg(QUrl::fromLocalFile(imagePath).toString(QUrl::FullyEncoded).toHtmlEscaped());
    }

    return QString("<a class=\"tile\" href=\"%1\"><div class=\"name\">%2</div>%3</a>\n")
           .arg(directoryNavigationUrl(dir.absoluteFilePath()).toHtmlEscaped(),
                label.toHtmlEscaped(),
                preview);
}

QString DirectoryWebView::directoryNavigationUrl(const QString &path) const
{
    QUrl url;
    QUrlQuery query;

    url.setScheme("zcp-directory");
    url.setHost("open");
    query.addQueryItem("path", path);
    url.setQuery(query);

    return url.toString(QUrl::FullyEncoded);
}

QString DirectoryWebView::thumbnailPath(const QString &rootPath, const QFileInfo &dir) const
{
    QString baseName = dir.baseName();
    QString path = findEntryThumbnail(rootPath, baseName);

    if (!path.isEmpty())
        return path;

    QString metadataFile = rootPath + "/" + METADATA_DIR + "/" + METADATA_FILE;

    if (QFile::exists(metadataFile))
    {
        QSet<QString> visited;
        path = findIncludedThumbnail(MetadataCache::get()->metadata(rootPath), baseName, &visited);

        if (!path.isEmpty())
            return path;
    }

    QString metaDir = dir.absoluteFilePath() + "/" + METADATA_DIR;

    path = findImageFile(metaDir, "01");

    if (!path.isEmpty())
        return path;

    path = metaDir + "/" + LOGO_TEXT_FILE;

    if (QFile::exists(path))
        return path;

    path = metaDir + "/" + LOGO_FILE;

    if (QFile::exists(path))
        return path;

    return QString();
}
