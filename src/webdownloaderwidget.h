#ifndef WEBDOWNLOADERWIDGET_H
#define WEBDOWNLOADERWIDGET_H

#include <QWidget>
#include <QWebEngineDownloadRequest>

namespace Ui {
class WebDownloaderWidget;
}

class WebDownloaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebDownloaderWidget(QWebEngineDownloadRequest *download, QWidget *parent = 0);
    ~WebDownloaderWidget();

private:
    Ui::WebDownloaderWidget *ui;

    QWebEngineDownloadRequest *m_download;

private slots:
    void stateChange(QWebEngineDownloadRequest::DownloadState state);
    void abort();
    void downloadProgress(qint64 received, qint64 total);

};

#endif // WEBDOWNLOADERWIDGET_H
