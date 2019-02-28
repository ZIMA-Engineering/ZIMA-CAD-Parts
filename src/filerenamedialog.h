#ifndef FILERENAMEDIALOG_H
#define FILERENAMEDIALOG_H

#include <QDialog>
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

public slots:
	void rename();

private:
	Ui::FileRenameDialog *ui;
	QString m_dir;
	QFileInfo m_file;
};

#endif // FILERENAMEDIALOG_H
