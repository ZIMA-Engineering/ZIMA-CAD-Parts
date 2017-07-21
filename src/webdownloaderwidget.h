#ifndef WEBDOWNLOADERWIDGET_H
#define WEBDOWNLOADERWIDGET_H

#include <QWidget>
#include <QWebEngineDownloadItem>

namespace Ui {
class WebDownloaderWidget;
}

class WebDownloaderWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WebDownloaderWidget(QWebEngineDownloadItem *download, QWidget *parent = 0);
	~WebDownloaderWidget();

private:
	Ui::WebDownloaderWidget *ui;

	QWebEngineDownloadItem *m_download;

private slots:
	void stateChange(QWebEngineDownloadItem::DownloadState state);
	void abort();
	void downloadProgress(qint64 received, qint64 total);

};

#endif // WEBDOWNLOADERWIDGET_H
