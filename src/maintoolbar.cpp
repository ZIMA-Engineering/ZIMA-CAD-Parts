#include "maintoolbar.h"
#include "ui_maintoolbar.h"
#include "languageflagswidget.h"
#include "workingdirwidget.h"
#include "metadata.h"
#include "datasourcehistory.h"

#include <QStyle>

MainToolBar::MainToolBar(QWidget *parent) :
    QToolBar(parent),
    ui(new Ui::MainToolBar),
    m_dsHistory(nullptr)
{
    ui->setupUi(this);

    ui->actionRefresh->setShortcut(QKeySequence::Refresh);
    ui->actionRefresh->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    connect(ui->actionRefresh, SIGNAL(triggered()),
            MetadataCache::get(), SLOT(clear()));

    ui->actionHistoryBack->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->actionHistoryForward->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));

    m_wdirWidget = new WorkingDirWidget(this);
    addWidget(m_wdirWidget);

    addSeparator();

    LanguageFlagsWidget *flagsWidget = new LanguageFlagsWidget(this);
    addWidget(flagsWidget);

    addSeparator();
    addAction(ui->actionSettings);
    connect(ui->actionSettings, SIGNAL(triggered()),
            this, SIGNAL(settingsRequested()));
}

MainToolBar::~MainToolBar()
{
    delete ui;
}

void MainToolBar::settingsChanged()
{
    m_wdirWidget->settingsChanged();
}

void MainToolBar::setupHistory(DataSourceHistory *history)
{
    if (m_dsHistory != nullptr)
    {
        disconnect(ui->actionHome, SIGNAL(triggered()),
                   m_dsHistory, SLOT(goToWorkingDirectory()));
        disconnect(ui->actionHistoryBack, SIGNAL(triggered()),
                   m_dsHistory, SLOT(goBack()));
        disconnect(ui->actionHistoryForward, SIGNAL(triggered()),
                   m_dsHistory, SLOT(goForward()));
        disconnect(m_dsHistory, SIGNAL(canGoBackChanged(bool)),
                   this, SLOT(canGoBackChange(bool)));
        disconnect(history, SIGNAL(canGoForwardChanged(bool)),
                   this, SLOT(canGoForwardChange(bool)));
    }

    m_dsHistory = history;

    connect(ui->actionHome, SIGNAL(triggered()),
            history, SLOT(goToWorkingDirectory()));
    connect(ui->actionHistoryBack, SIGNAL(triggered()),
            history, SLOT(goBack()));
    connect(ui->actionHistoryForward, SIGNAL(triggered()),
            history, SLOT(goForward()));
    connect(history, SIGNAL(canGoBackChanged(bool)),
            this, SLOT(canGoBackChange(bool)));
    connect(history, SIGNAL(canGoForwardChanged(bool)),
            this, SLOT(canGoForwardChange(bool)));

    ui->actionHistoryBack->setEnabled(history->canGoBack());
    ui->actionHistoryForward->setEnabled(history->canGoForward());
}

void MainToolBar::canGoBackChange(bool can)
{
    ui->actionHistoryBack->setEnabled(can);
}

void MainToolBar::canGoForwardChange(bool can)
{
    ui->actionHistoryForward->setEnabled(can);
}
