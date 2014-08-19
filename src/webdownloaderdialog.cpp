#include "webdownloaderdialog.h"
#include "ui_webdownloaderdialog.h"

#include <QNetworkReply>
#include <QHBoxLayout>


WebDownloaderDialog::WebDownloaderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebDownloaderDialog)
{
    ui->setupUi(this);

    m_layout = new QVBoxLayout();
    ui->scrollArea->setLayout(m_layout);
    m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

WebDownloaderDialog::~WebDownloaderDialog()
{
    delete ui;
}

void WebDownloaderDialog::enqueue(const QString &fileName, QNetworkReply *reply)
{
    WebDownloaderWidget *w = new WebDownloaderWidget(fileName, reply, this);
    m_map[fileName] = w;
    // always insert w at the latest position before spacer
    m_layout->insertWidget(m_layout->count()-2, w);
}
