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
	bool editNext() const;

public slots:
	void saveAndNext();
	void save();

private:
	Ui::FileEditDialog *ui;
	QString m_dir;
	QString m_file;
	QList<QLineEdit*> m_edits;
	bool m_next;
};

#endif // FILEEDITDIALOG_H
