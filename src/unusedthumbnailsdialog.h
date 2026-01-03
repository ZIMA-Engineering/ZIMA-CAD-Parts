#ifndef UNUSEDTHUMBNAILSDIALOG_H
#define UNUSEDTHUMBNAILSDIALOG_H

#include <QDialog>
#include <QFileInfoList>

namespace Ui {
class UnusedThumbnailsDialog;
}

class UnusedThumbnailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnusedThumbnailsDialog(const QString &directory, const QFileInfoList &thumbnails, QWidget *parent = nullptr);
    ~UnusedThumbnailsDialog();

private:
    Ui::UnusedThumbnailsDialog *ui;
};

#endif // UNUSEDTHUMBNAILSDIALOG_H
