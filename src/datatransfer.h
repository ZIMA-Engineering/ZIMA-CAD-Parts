#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include <QObject>
#include <QIODevice>

class DataTransfer : public QObject
{
	Q_OBJECT
public:
	explicit DataTransfer(QIODevice *src, QIODevice *dst, QObject *parent = 0);

signals:
	void done();

private slots:
	void onReadingChannelClosed();
	void onReadyRead();

private:
	QIODevice *src;
	QIODevice *dst;

};

#endif // DATATRANSFER_H
