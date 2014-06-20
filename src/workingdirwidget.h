#ifndef WORKINGDIRWIDGET_H
#define WORKINGDIRWIDGET_H

#include <QWidget>

namespace Ui {
class WorkingDirWidget;
}


/*!
 * \brief The web-browser-like line edit widget to display up-to-date working dir
 *
 * Synchronized with Settings::WorkingDir
 */
class WorkingDirWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WorkingDirWidget(QWidget *parent = 0);
	~WorkingDirWidget();

	void settingsChanged();

private:
	Ui::WorkingDirWidget *ui;

private slots:
	void openWorkingDirectory();
	void setWorkingDirectoryDialog();

};

#endif // WORKINGDIRWIDGET_H
