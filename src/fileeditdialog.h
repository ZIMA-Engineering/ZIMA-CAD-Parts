#ifndef FILEEDITDIALOG_H
#define FILEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>

namespace Ui {
class FileEditDialog;
}

class FileEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FileEditDialog(QString dir, QString file, QWidget *parent = 0);
	~FileEditDialog();

public slots:
	void save();

private:
	Ui::FileEditDialog *ui;
	QString m_dir;
	QString m_file;
	QList<QLineEdit*> m_edits;
};

#endif // FILEEDITDIALOG_H
