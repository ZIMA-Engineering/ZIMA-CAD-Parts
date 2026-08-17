#include "filerenamedialog.h"
#include "ui_filerenamedialog.h"
#include "filerenamer.h"
#include "file.h"

#include <QMessageBox>

FileRenameDialog::FileRenameDialog(QString dir, QFileInfo file, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileRenameDialog),
    m_dir(dir),
    m_file(file),
    m_counter(0)
{
    ui->setupUi(this);

    setWindowTitle(tr("Rename %1").arg(File::partBaseName(file)));

    if (m_file.isDir()) {
        ui->fileNameLabel->setText(tr("Directory name"));
    }

    ui->fileNameLineEdit->setText(File::partBaseName(file));

    connect(ui->renameButton, SIGNAL(clicked(bool)),
            this, SLOT(rename()));
    connect(ui->cancelButton, SIGNAL(clicked(bool)),
            this, SLOT(reject()));
    connect(ui->closeButton, SIGNAL(clicked(bool)),
            this, SLOT(accept()));

    ui->logWidget->hide();
    ui->closeButton->hide();
}

FileRenameDialog::~FileRenameDialog()
{
    delete ui;
}

void FileRenameDialog::rename()
{
    const QString newName = ui->fileNameLineEdit->text().trimmed();
    QString validationError;
    if (!File::isValidPartName(newName, &validationError))
    {
        QMessageBox::warning(this, tr("Invalid name"), validationError);
        return;
    }

    FileRenamer rn;

    connect(&rn, SIGNAL(renamed(QFileInfo,QFileInfo)),
            this, SLOT(fileRenamed(QFileInfo,QFileInfo)));
    connect(&rn, SIGNAL(error(QFileInfo,QFileInfo,QString)),
            this, SLOT(fileError(QFileInfo,QFileInfo,QString)));

    ui->cancelButton->hide();
    ui->renameButton->hide();

    ui->closeButton->setText(tr("Renaming files..."));
    ui->closeButton->setDisabled(true);
    ui->closeButton->show();

    if (rn.rename(m_dir.absolutePath(), m_file, newName)) {
        accept();
    } else {
        ui->logWidget->show();
        ui->logTextEdit->append(tr("Renamed %1 files and failed.").arg(m_counter));
        ui->closeButton->setEnabled(true);
        ui->closeButton->setText(tr("Close"));
    }
}

void FileRenameDialog::fileRenamed(const QFileInfo &oldFile, const QFileInfo &newFile)
{
    ui->logTextEdit->append(
        tr("Renamed \"%1\" to \"%2\"\n")
        .arg(m_dir.relativeFilePath(oldFile.absoluteFilePath()))
        .arg(m_dir.relativeFilePath(newFile.absoluteFilePath()))
    );

    m_counter++;
}

void FileRenameDialog::fileError(const QFileInfo &oldFile, const QFileInfo &newFile, const QString &error)
{
    ui->logTextEdit->append(
        tr("Unable to rename \"%1\" to \"%2\": %3\n")
        .arg(m_dir.relativeFilePath(oldFile.absoluteFilePath()))
        .arg(m_dir.relativeFilePath(newFile.absoluteFilePath()))
        .arg(error)
    );

    ui->closeButton->setEnabled(true);
    ui->closeButton->setText(tr("Close"));
}
