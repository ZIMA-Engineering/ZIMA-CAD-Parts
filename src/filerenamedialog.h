#ifndef FILERENAMEDIALOG_H
#define FILERENAMEDIALOG_H

#include <QDialog>
#include <QDir>
#include <QFileInfo>

namespace Ui {
class FileRenameDialog;
}

class FileRenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileRenameDialog(QString dir, QFileInfo file, QWidget *parent = 0);
    ~FileRenameDialog();

private slots:
    void rename();
    void fileRenamed(const QFileInfo &oldFile, const QFileInfo &newFile);
    void fileError(const QFileInfo &oldFile, const QFileInfo &newFile, const QString &error);

private:
    Ui::FileRenameDialog *ui;
    QDir m_dir;
    QFileInfo m_file;
    int m_counter;
};

#endif // FILERENAMEDIALOG_H
