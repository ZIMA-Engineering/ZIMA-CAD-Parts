#ifndef WEBDOWNLOADERWIDGET_H
#define WEBDOWNLOADERWIDGET_H

#include <QWidget>
#include <QNetworkReply>

class QFile;

namespace Ui {
class WebDownloaderWidget;
}

class WebDownloaderWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WebDownloaderWidget(const QString &fileName, QNetworkReply *reply, QWidget *parent = 0);
	~WebDownloaderWidget();

private:
	Ui::WebDownloaderWidget *ui;

	QNetworkReply *m_reply;
	QFile *m_file;

private slots:
	void error(QNetworkReply::NetworkError code);
	void abort();
	void finished();
	void downloadProgress(qint64 received, qint64 total);

};

#endif // WEBDOWNLOADERWIDGET_H
