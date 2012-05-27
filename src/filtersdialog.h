#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include <QDialog>
#include <QList>
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>

#include "mainwindow.h"

namespace Ui {
class FiltersDialog;
}

class FiltersDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit FiltersDialog(QWidget *parent = 0);
	~FiltersDialog();
	void accept();
	
private:
	struct GroupCheckBox
	{
		QGroupBox *groupBox;
		QList<QCheckBox*> checkBoxes;
	};

	Ui::FiltersDialog *ui;
	QList<GroupCheckBox*> groups;
};

#endif // FILTERSDIALOG_H
