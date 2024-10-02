#include "directorycopyasdialog.h"
#include "ui_directorycopyasdialog.h"

DirectoryCopyAsDialog::DirectoryCopyAsDialog(const QFileInfo &sourceDirectory, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DirectoryCopyAsDialog),
    m_sourceDirectory(sourceDirectory)
{
    ui->setupUi(this);

    ui->pathLabel->setText(sourceDirectory.absolutePath());
    ui->nameLineEdit->setText(sourceDirectory.baseName());
}

DirectoryCopyAsDialog::~DirectoryCopyAsDialog()
{
    delete ui;
}

QString DirectoryCopyAsDialog::directoryPath()
{
    return m_sourceDirectory.absolutePath() + "/" + ui->nameLineEdit->text();
}
