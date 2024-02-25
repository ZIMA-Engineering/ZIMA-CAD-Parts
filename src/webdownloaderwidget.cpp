#include "webdownloaderwidget.h"
#include "ui_webdownloaderwidget.h"

WebDownloaderWidget::WebDownloaderWidget(QWebEngineDownloadRequest *download, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::WebDownloaderWidget),
	m_download(download)
{
	ui->setupUi(this);

	ui->fnameLabel->setText(
		QString("%1/%2").arg(download->downloadDirectory()).arg(download->downloadFileName())
	);

	ui->progressBar->setMinimum(0);
	ui->progressBar->setMaximum(download->totalBytes());
	ui->progressBar->show();

	connect(ui->abortButton, SIGNAL(clicked()),
			this, SLOT(abort()));

	connect(m_download, SIGNAL(downloadProgress(qint64,qint64)),
			this, SLOT(downloadProgress(qint64,qint64)));
	connect(m_download, SIGNAL(stateChanged(QWebEngineDownloadRequest::DownloadState)),
			this, SLOT(stateChange(QWebEngineDownloadRequest::DownloadState)));
}

WebDownloaderWidget::~WebDownloaderWidget()
{
	delete ui;
}

void WebDownloaderWidget::stateChange(QWebEngineDownloadRequest::DownloadState state)
{
	switch (state) {
	case QWebEngineDownloadRequest::DownloadCancelled:
		ui->statusLabel->setText("Cancelled by user");
		break;

	case QWebEngineDownloadRequest::DownloadCompleted:
		ui->progressBar->hide();
		ui->abortButton->hide();
		ui->statusLabel->setText(tr("Download finished"));
		break;

	case QWebEngineDownloadRequest::DownloadInterrupted:
		ui->progressBar->hide();
		ui->statusLabel->setText(tr("Error, download interrupted"));
		break;

	default:
		return;
	}
}

void WebDownloaderWidget::abort()
{
	ui->statusLabel->setText("Cancelling...");
	ui->abortButton->hide();
	m_download->cancel();
}

void WebDownloaderWidget::downloadProgress(qint64 received, qint64 total)
{
	ui->progressBar->setMaximum(total);
	ui->progressBar->setValue(received);
}
