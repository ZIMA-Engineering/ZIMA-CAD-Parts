#ifndef WEBDOWNLOADDIALOG_H
#define WEBDOWNLOADDIALOG_H

#include <QWidget>

namespace Ui {
class WebDownloadDialog;
}

class WebDownloadDialog : public QWidget
{
	Q_OBJECT

public:
	explicit WebDownloadDialog(QWidget *parent = 0);
	~WebDownloadDialog();

private:
	Ui::WebDownloadDialog *ui;
};

#endif // WEBDOWNLOADDIALOG_H
