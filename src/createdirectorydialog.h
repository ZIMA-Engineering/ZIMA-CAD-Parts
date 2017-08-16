#ifndef CREATEDIRECTORYDIALOG_H
#define CREATEDIRECTORYDIALOG_H

#include <QDialog>

namespace Ui {
class CreateDirectoryDialog;
}

class CreateDirectoryDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CreateDirectoryDialog(const QString &path, QWidget *parent = 0);
	~CreateDirectoryDialog();
	QString name() const;
	bool hasPrototype() const;
	QString prototype() const;

private:
	Ui::CreateDirectoryDialog *ui;
	QString m_path;

	void disablePrototype();

private slots:
	void nameChange(const QString &text);
};

#endif // CREATEDIRECTORYDIALOG_H
