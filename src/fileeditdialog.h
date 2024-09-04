#ifndef FILEEDITDIALOG_H
#define FILEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QFileInfo>

namespace Ui {
class FileEditDialog;
}

class FileModel;

class FileEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileEditDialog(QFileInfo file, FileModel *fileModel, QWidget *parent = 0);
    ~FileEditDialog();
    bool editNext() const;

public slots:
    void saveAndNext();
    void save();

private:
    Ui::FileEditDialog *ui;
    QString m_dir;
    QString m_fileName;
    QFileInfo m_fileInfo;
    FileModel *m_fileModel;
    QString m_thumbnailPath;
    QString m_origThumbnailPath;
    QList<QLineEdit*> m_edits;
    bool m_next;

    void setupThumbnail();
    void installThumbnail();
    void uninstallThumbnail();
    void setThumbnail(const QPixmap &thumbnail);

private slots:
    void removeThumbnail();
    void openThumbnailDialog();
};

#endif // FILEEDITDIALOG_H
