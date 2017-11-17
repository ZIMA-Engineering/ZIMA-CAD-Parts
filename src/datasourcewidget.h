#ifndef DATASOURCEWIDGET_H
#define DATASOURCEWIDGET_H

#include "ui_datasourcewidget.h"
#include "settingsdialog.h"
#include "directorywidget.h"

class QSignalMapper;


/*!
 * \brief The data source tree and ServerTabWidget user interface.
 *
 * The data source tree is kept in NavBar
 */
class DataSourceWidget : public QWidget, public Ui::DataSourceWidget
{
	Q_OBJECT
public:
	explicit DataSourceWidget(QWidget *parent = 0);

	QModelIndex currentIndex();
	void setDirectory(const QString &path);

signals:
	void showSettings(SettingsDialog::Section);

	void itemLoaded(const QModelIndex&);
	void clicked(const QModelIndex&);
	void activated(const QModelIndex&);

	void techSpecAvailable(const QUrl&);

	void workingDirChanged();
	void directorySelected(const QString&);

public slots:
	void expand(const QModelIndex & index);
	void settingsChanged();
	void goToWorkingDirectory();

private:
	QStringList m_zimaUtils;

private slots:
	void splitterMoved(int, int);
	void handleOpenPartDirectory(const QFileInfo &fi);
};

#endif // DATASOURCEWIDGET_H
