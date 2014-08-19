#include "webdowloaderdialog.h"
#include "ui_webdowloaderdialog.h"

#include <QNetworkReply>
#include <QHBoxLayout>


WebDowloaderDialog::WebDowloaderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebDowloaderDialog)
{
    ui->setupUi(this);

    m_layout = new QVBoxLayout();
    ui->scrollArea->setLayout(m_layout);
    m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

WebDowloaderDialog::~WebDowloaderDialog()
{
    delete ui;
}

void WebDowloaderDialog::enqueue(const QString &fileName, QNetworkReply *reply)
{
    WebDownloaderWidget *w = new WebDownloaderWidget(fileName, reply, this);
    m_map[fileName] = w;
    // always insert w at the latest position before spacer
    m_layout->insertWidget(m_layout->count()-2, w);
}
