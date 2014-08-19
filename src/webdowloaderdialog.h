#ifndef WEBDOWLOADERDIALOG_H
#define WEBDOWLOADERDIALOG_H

#include <QDialog>
#include <QHash>

#include "webdownloaderwidget.h"


class QVBoxLayout;
class QNetworkReply;

namespace Ui {
class WebDowloaderDialog;
}

class WebDowloaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WebDowloaderDialog(QWidget *parent = 0);
    ~WebDowloaderDialog();

    void enqueue(const QString &fileName, QNetworkReply *reply);

private:
    Ui::WebDowloaderDialog *ui;

    QHash<QString,WebDownloaderWidget*> m_map;
    QVBoxLayout *m_layout;
};

#endif // WEBDOWLOADERDIALOG_H
