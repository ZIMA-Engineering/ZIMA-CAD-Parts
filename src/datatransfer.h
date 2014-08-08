#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include <QObject>
#include <QNetworkReply>

struct File;

#if 0
class DataTransfer : public QObject
{
	Q_OBJECT
public:
	explicit DataTransfer(QNetworkReply *src, File *dst, QObject *parent = 0);
	~DataTransfer();
	bool initiate() const;
	void setSource(QNetworkReply *src);
	void setDeleteSrc(bool d);
	void setDeleteDst(bool d);

public slots:
	void cancel();

signals:
	void progress(File *f);
	void done(File *f);

private slots:
	void onReadingChannelClosed();
	void onReadyRead();
	void onMetadataChange();

private:
	QNetworkReply *src;
	File *dst;
	bool deleteSrc;
	bool deleteDst;
	bool canceled;
};

#endif
#endif // DATATRANSFER_H
