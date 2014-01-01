#include "datatransfer.h"

#include <QDebug>

DataTransfer::DataTransfer(QIODevice *src, QIODevice *dst, QObject *parent) :
	QObject(parent),
	src(src),
	dst(dst)
{
	connect(src, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(src, SIGNAL(readChannelFinished()), this, SLOT(onReadingChannelClosed()));
}

void DataTransfer::onReadingChannelClosed()
{
	onReadyRead();
	dst->close();

	qDebug() << "Download done";

	emit done();
	deleteLater();
}

void DataTransfer::onReadyRead()
{
	while(src->bytesAvailable())
		dst->write(src->read(4096));
}
