#ifndef MAINTABWIDGET_H
#define MAINTABWIDGET_H

#include <QTabWidget>

#include "settingsdialog.h"

class DataSourceWidget;
class DataSourceHistory;

namespace Ui {
class MainTabWidget;
}

class MainTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	explicit MainTabWidget(QWidget *parent = 0);
	~MainTabWidget();
	DataSourceWidget* currentDataSource();
	DataSourceWidget* dataSourceAt(int index);

public slots:
	void goToWorkingDirectory();
	void settingsChanged();

signals:
	void showSettings(SettingsDialog::Section);
	void workingDirChanged();
	void newHistory(DataSourceHistory *history);

private:
	Ui::MainTabWidget *ui;

	void addDataSourceWidget(const QString &dir);

private slots:
	void addNewTab();
	void closeTab(int index);
	void tabChange(int index);
	void updateTabTitle(DataSourceWidget *dsw, const QString &dir);
};

#endif // MAINTABWIDGET_H
