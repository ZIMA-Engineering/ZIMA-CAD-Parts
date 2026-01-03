#include "unusedthumbnailsdialog.h"
#include "ui_unusedthumbnailsdialog.h"

#include <QDir>

UnusedThumbnailsDialog::UnusedThumbnailsDialog(const QString &directory, const QFileInfoList &thumbnails, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnusedThumbnailsDialog)
{
    ui->setupUi(this);

    ui->introLabel->setText(
        tr("The following %n unused thumbnails were found. Delete them?", "", thumbnails.size())
    );
    ui->directoryValueLabel->setText(QDir::toNativeSeparators(directory));

    QDir baseDir(directory);

    foreach (const QFileInfo &thumb, thumbnails)
        ui->thumbnailsTextEdit->appendPlainText(baseDir.relativeFilePath(thumb.absoluteFilePath()));
}

UnusedThumbnailsDialog::~UnusedThumbnailsDialog()
{
    delete ui;
}
