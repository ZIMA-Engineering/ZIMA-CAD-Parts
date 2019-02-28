#include "filerenamedialog.h"
#include "ui_filerenamedialog.h"
#include "filerenamer.h"

FileRenameDialog::FileRenameDialog(QString dir, QFileInfo file, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FileRenameDialog),
	m_dir(dir),
	m_file(file)
{
	ui->setupUi(this);

	setWindowTitle(tr("Rename %1").arg(file.baseName()));
	ui->fileNameLineEdit->setText(file.baseName());
}

FileRenameDialog::~FileRenameDialog()
{
	delete ui;
}

void FileRenameDialog::rename()
{
	FileRenamer rn;
	rn.rename(m_dir, m_file, ui->fileNameLineEdit->text());
}
