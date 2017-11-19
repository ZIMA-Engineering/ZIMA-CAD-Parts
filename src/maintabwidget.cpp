#include "maintabwidget.h"
#include "ui_maintabwidget.h"
#include "datasourcewidget.h"
#include "settings.h"

#include <QDebug>
#include <QPushButton>

MainTabWidget::MainTabWidget(QWidget *parent) :
	QTabWidget(parent),
	ui(new Ui::MainTabWidget)
{
	ui->setupUi(this);

	addDataSourceWidget(Settings::get()->getWorkingDir());

	auto addTabBtn = new QPushButton(QIcon(":/gfx/list-add.png"), QString(), this);

	connect(addTabBtn, SIGNAL(clicked()), this, SLOT(addNewTab()));

	setCornerWidget(addTabBtn, Qt::TopLeftCorner);

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChange(int)));
}

MainTabWidget::~MainTabWidget()
{
	delete ui;
}

DataSourceWidget *MainTabWidget::currentDataSource()
{
	return static_cast<DataSourceWidget*>(currentWidget());
}

DataSourceWidget *MainTabWidget::dataSourceAt(int index)
{
	return static_cast<DataSourceWidget*>(widget(index));
}

void MainTabWidget::goToWorkingDirectory()
{
	currentDataSource()->goToWorkingDirectory();
}

void MainTabWidget::settingsChanged()
{
	int cnt = count();

	for (int i = 0; i < cnt; i++)
		dataSourceAt(i)->settingsChanged();
}

void MainTabWidget::addDataSourceWidget(const QString &dir)
{
	auto dsw = new DataSourceWidget(this);

	connect(dsw, SIGNAL(showSettings(SettingsDialog::Section)),
			this, SIGNAL(showSettings(SettingsDialog::Section)));
	connect(dsw, SIGNAL(directoryChanged(DataSourceWidget*, QString)),
			this, SLOT(updateTabTitle(DataSourceWidget*, QString)));
	connect(dsw, SIGNAL(workingDirChanged()),
			this, SIGNAL(workingDirChanged()));

	int i = addTab(dsw, QFileInfo(dir).baseName());

	if (count() > 1)
		setTabsClosable(true);

	setCurrentIndex(i);
	dsw->settingsChanged();
}

void MainTabWidget::addNewTab()
{
	addDataSourceWidget(Settings::get()->getWorkingDir());
}

void MainTabWidget::closeTab(int index)
{
	if (count() <= 1)
		setTabsClosable(false);

	removeTab(index);
}

void MainTabWidget::tabChange(int index)
{
	emit newHistory(dataSourceAt(index)->history());
}

void MainTabWidget::updateTabTitle(DataSourceWidget *dsw, const QString &dir)
{
	setTabText(
		indexOf(dsw),
		QFileInfo(dir).baseName()
	);
}
