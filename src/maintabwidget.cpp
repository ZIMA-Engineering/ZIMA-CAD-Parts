#include "maintabwidget.h"
#include "ui_maintabwidget.h"
#include "datasourcewidget.h"
#include "settings.h"

#include <QDebug>
#include <QPushButton>

MainTabWidget::MainTabWidget(QWidget *parent) :
	QTabWidget(parent),
	ui(new Ui::MainTabWidget),
	m_loading(true)
{
	ui->setupUi(this);

	load();

	auto addTabBtn = new QPushButton(QIcon(":/gfx/list-add.png"), QString(), this);

	connect(addTabBtn, SIGNAL(clicked()), this, SLOT(addNewTab()));

	setCornerWidget(addTabBtn, Qt::TopLeftCorner);

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChange(int)));
	connect(tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(save()));
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

void MainTabWidget::openInANewTab(const QString &dir)
{
	addDataSourceWidget(dir);
}

void MainTabWidget::tabInserted(int index)
{
	Q_UNUSED(index)

	if (m_loading)
		return;

	save();
}

void MainTabWidget::tabRemoved(int index)
{
	Q_UNUSED(index)

	if (m_loading)
		return;

	save();
}

void MainTabWidget::load()
{
	QStringList tabDirs = Settings::get()->MainTabs;
	m_loading = true;

	if (tabDirs.empty())
	{
		addDataSourceWidget(Settings::get()->getWorkingDir());
		return;
	}

	foreach (const QString &dir, tabDirs)
		addDataSourceWidget(dir);

	setCurrentIndex(Settings::get()->ActiveMainTab);

	m_loading = false;
}

void MainTabWidget::addDataSourceWidget(const QString &dir)
{
	auto dsw = new DataSourceWidget(dir, this);

	connect(dsw, SIGNAL(showSettings(SettingsDialog::Section)),
			this, SIGNAL(showSettings(SettingsDialog::Section)));
	connect(dsw, SIGNAL(directoryChanged(DataSourceWidget*, QString)),
			this, SLOT(updateTabTitle(DataSourceWidget*, QString)));
	connect(dsw, SIGNAL(directoryChanged(DataSourceWidget*,QString)),
			this, SLOT(save()));
	connect(dsw, SIGNAL(workingDirChanged()),
			this, SIGNAL(workingDirChanged()));
	connect(dsw, SIGNAL(openInANewTabRequested(QString)),
			this, SLOT(openInANewTab(QString)));

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
	save();
	emit newHistory(dataSourceAt(index)->history());
}

void MainTabWidget::updateTabTitle(DataSourceWidget *dsw, const QString &dir)
{
	setTabText(
		indexOf(dsw),
		QFileInfo(dir).baseName()
	);
}

void MainTabWidget::save()
{
	QStringList tabDirs;
	int cnt = count();

	for (int i = 0; i < cnt; i++)
		tabDirs << dataSourceAt(i)->currentDir();

	Settings::get()->setMainTabs(tabDirs, currentIndex());
}
