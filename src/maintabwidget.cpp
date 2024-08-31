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

    auto addTabBtn = new QToolButton(this);
    addTabBtn->setIcon(QIcon(":/gfx/list-add.png"));
    addTabBtn->setToolTip(tr("Open a new tab"));
    connect(addTabBtn, SIGNAL(clicked()), this, SLOT(addNewTab()));

    int i = addTab(new QLabel("Add tabs by pressing \"+\""), QString());
    setTabEnabled(i, false);
    tabBar()->setTabButton(i, QTabBar::RightSide, addTabBtn);

    load();

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChange(int)));
    connect(tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(handleTabMove(int,int)));
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
    int cnt = count() - 1;

    for (int i = 0; i < cnt; i++)
        dataSourceAt(i)->settingsChanged();
}

void MainTabWidget::openInANewTab(const QString &dir)
{
    addDataSourceWidget(dir);
}

void MainTabWidget::openAboutPage()
{
    currentDataSource()->openAboutPage();
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

    int i = insertTab(qMax(count() - 1, 0), dsw, "<you should not see this>");
    updateTabTitle(dsw, dir);

    if (count() > 2)
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
    if (count() <= 2)
        setTabsClosable(false);

    removeTab(index);
}

void MainTabWidget::tabChange(int index)
{
    if (index == count()-1)
    {
        // The last tab is the add tab button, so activate
        // the previous tab instead.
        setCurrentIndex(qMax(index-1, 0));
        return;
    }

    save();
    emit newHistory(dataSourceAt(index)->history());
}

void MainTabWidget::updateTabTitle(DataSourceWidget *dsw, const QString &dir)
{
    int i = indexOf(dsw);
    QString label = MetadataCache::get()->label(dir);
    QString logoPath = dir + "/" + METADATA_DIR + "/";

    if (QFile::exists(logoPath + LOGO_TEXT_FILE))
        setTabIcon(i, QIcon(logoPath + LOGO_TEXT_FILE));

    setTabText(
        i,
        label.isEmpty() ? QFileInfo(dir).baseName() : label
    );
}

void MainTabWidget::handleTabMove(int from, int to)
{
    Q_UNUSED(to)
    /*
     * The last tab has to be with the add tab button, so when the last
     * tab is moved, put it immediately back in place.
     */
    if (from == count()-1)
        tabBar()->moveTab(to, count()-1);

    save();
}

void MainTabWidget::save()
{
    QStringList tabDirs;
    int cnt = count() - 1;

    for (int i = 0; i < cnt; i++)
    {
        qDebug() << "Save tab" << i << tabText(i);
        tabDirs << dataSourceAt(i)->currentDir();
    }

    Settings::get()->setMainTabs(tabDirs, currentIndex());
}
