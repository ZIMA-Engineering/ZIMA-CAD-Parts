#ifndef DIRECTORYCOPYASDIALOG_H
#define DIRECTORYCOPYASDIALOG_H

#include <QDialog>
#include <QFileInfo>

namespace Ui {
class DirectoryCopyAsDialog;
}

class DirectoryCopyAsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirectoryCopyAsDialog(const QFileInfo &sourceDirectory, QWidget *parent = nullptr);
    ~DirectoryCopyAsDialog();
    QString directoryPath();

private:
    Ui::DirectoryCopyAsDialog *ui;
    QFileInfo m_sourceDirectory;
};

#endif // DIRECTORYCOPYASDIALOG_H
