#ifndef DATASOURCEWIDGET_H
#define DATASOURCEWIDGET_H

#include "ui_datasourcewidget.h"
#include "settingsdialog.h"
#include "directorywidget.h"

class QSignalMapper;
class DataSourceHistory;


/*!
 * \brief The data source tree and ServerTabWidget user interface.
 *
 * The data source tree is kept in NavBar
 */
class DataSourceWidget : public QWidget, public Ui::DataSourceWidget
{
	Q_OBJECT
public:
	explicit DataSourceWidget(const QString &dir, QWidget *parent = 0);

	QModelIndex currentIndex();
	DataSourceHistory* history();
	QString currentDir() const;

signals:
	void showSettings(SettingsDialog::Section);

	void itemLoaded(const QModelIndex&);
	void clicked(const QModelIndex&);
	void activated(const QModelIndex&);

	void techSpecAvailable(const QUrl&);

	void workingDirChanged();
	void directoryChanged(DataSourceWidget*, const QString&);

	void openInANewTabRequested(const QString &path);

public slots:
	void expand(const QModelIndex & index);
	void settingsChanged();
	void setDirectory(const QString &path);
	void goToWorkingDirectory();

private:
	DataSourceHistory *m_history;
	QStringList m_zimaUtils;
	QString m_currentDir;

private slots:
	void setupDataSources(const QString &dir);
	void splitterMoved(int, int);
	void handleOpenPartDirectory(const QFileInfo &fi);
	void announceDirectoryChange(const QString &dir);
};

#endif // DATASOURCEWIDGET_H
