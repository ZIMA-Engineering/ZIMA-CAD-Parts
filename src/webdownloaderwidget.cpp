#include "webdownloaderwidget.h"
#include "ui_webdownloaderwidget.h"

#include <QNetworkReply>
#include <QFile>


WebDownloaderWidget::WebDownloaderWidget(const QString &fileName, QNetworkReply *reply, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::WebDownloaderWidget),
	m_reply(reply)
{
	ui->setupUi(this);

	ui->fnameLabel->setText(fileName);

	m_file = new QFile(fileName, this);
	if (!m_file->open(QIODevice::WriteOnly|QIODevice::Truncate))
	{
		ui->progressBar->hide();
		ui->abortButton->hide();
		ui->statusLabel->setText(tr("Error: %1").arg(tr("Cannot open file for writing")));
		abort();
	}
	else
	{
		ui->progressBar->setMinimum(0);
		ui->progressBar->setMaximum(reply->bytesAvailable());
		ui->progressBar->show();

		connect(ui->abortButton, SIGNAL(clicked()),
		        this, SLOT(abort()));

		connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
		        this, SLOT(downloadProgress(qint64,qint64)));
		connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
		        this, SLOT(error(QNetworkReply::NetworkError)));
		connect(m_reply, SIGNAL(finished()),
		        this, SLOT(finished()));
	}
}

WebDownloaderWidget::~WebDownloaderWidget()
{
	delete ui;
	m_reply->close();
	m_reply->deleteLater();
}

void WebDownloaderWidget::error(QNetworkReply::NetworkError code)
{
	Q_UNUSED(code)
	ui->progressBar->hide();
	ui->statusLabel->setText(tr("Error: %1").arg(m_reply->errorString()));
}

void WebDownloaderWidget::abort()
{
	ui->statusLabel->setText("Cancelled by user");
	ui->abortButton->hide();
	m_reply->abort();
}

void WebDownloaderWidget::finished()
{
	ui->progressBar->hide();
	ui->abortButton->hide();
	ui->statusLabel->setText(tr("Download finished"));
	m_file->close();
}

void WebDownloaderWidget::downloadProgress(qint64 received, qint64 total)
{
	ui->progressBar->setMaximum(total);
	ui->progressBar->setValue(received);

	m_file->write(m_reply->readAll());
	if (received == total)
		finished();
}
