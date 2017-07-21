#ifndef WEBDOWLOADERDIALOG_H
#define WEBDOWLOADERDIALOG_H

#include <QDialog>
#include <QWebEngineDownloadItem>

#include "webdownloaderwidget.h"


class QVBoxLayout;
class QNetworkReply;

namespace Ui {
class WebDownloaderDialog;
}

class WebDownloaderDialog : public QDialog
{
	Q_OBJECT

public:
	explicit WebDownloaderDialog(QWidget *parent = 0);
	~WebDownloaderDialog();

	void enqueue(QWebEngineDownloadItem *download);

private:
	Ui::WebDownloaderDialog *ui;
	QVBoxLayout *m_layout;
};

#endif // WEBDOWLOADERDIALOG_H
