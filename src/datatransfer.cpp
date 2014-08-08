#include "datatransfer.h"

#include <QDebug>
#include "item.h"

#if 0
DataTransfer::DataTransfer(QNetworkReply *src, File *dst, QObject *parent) :
	QObject(parent),
	dst(dst),
	deleteSrc(false),
	deleteDst(false),
	canceled(false)
{
	setSource(src);
}

DataTransfer::~DataTransfer()
{
	if(deleteSrc)
		src->deleteLater();

	if(deleteDst)
	{
		if(dst->openFtpFile)
			delete dst->openFtpFile;

		delete dst;
	}
}

bool DataTransfer::initiate() const
{
	dst->openFtpFile = new QFile(dst->targetPath);

	if(!dst->openFtpFile->open(QIODevice::WriteOnly))
	{
		qWarning() << "Failed to open file to download" << dst->targetPath;
		delete dst->openFtpFile;
		delete dst;
		return false;
	}

	return true;
}

void DataTransfer::setSource(QNetworkReply *src)
{
	this->src = src;
	canceled = false;

	connect(src, SIGNAL(metaDataChanged()), this, SLOT(onMetadataChange()));
	connect(src, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(src, SIGNAL(readChannelFinished()), this, SLOT(onReadingChannelClosed()));
}

void DataTransfer::setDeleteSrc(bool d)
{
	deleteSrc = d;
}

void DataTransfer::setDeleteDst(bool d)
{
	deleteDst = d;
}

void DataTransfer::cancel()
{
	canceled = true;
	src->close();
	dst->openFtpFile->close();
	delete dst->openFtpFile;
	dst->openFtpFile = 0;
}

void DataTransfer::onReadingChannelClosed()
{
	if(canceled)
		return;

	onReadyRead();
	dst->openFtpFile->close();

	qDebug() << "Download done";

	emit done(dst);
}

void DataTransfer::onReadyRead()
{
	QByteArray tmp;

	while(src->bytesAvailable())
	{
		tmp = src->read(4096);
		dst->bytesDone += tmp.size();

		dst->openFtpFile->write(tmp);
	}

	emit progress(dst);
}

void DataTransfer::onMetadataChange()
{
	if(src->hasRawHeader("Content-Length"))
		dst->size = src->rawHeader("Content-Length").toULongLong();
}

#endif
