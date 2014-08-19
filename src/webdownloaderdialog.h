#ifndef WEBDOWLOADERDIALOG_H
#define WEBDOWLOADERDIALOG_H

#include <QDialog>
#include <QHash>

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

    void enqueue(const QString &fileName, QNetworkReply *reply);

private:
    Ui::WebDownloaderDialog *ui;

    QHash<QString,WebDownloaderWidget*> m_map;
    QVBoxLayout *m_layout;
};

#endif // WEBDOWLOADERDIALOG_H
