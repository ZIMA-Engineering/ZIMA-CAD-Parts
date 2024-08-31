#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include <QToolBar>

class WorkingDirWidget;
class DataSourceHistory;

namespace Ui {
class MainToolBar;
}

class MainToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit MainToolBar(QWidget *parent = 0);
    ~MainToolBar();

public slots:
    void settingsChanged();
    void setupHistory(DataSourceHistory *history);

signals:
    void settingsRequested();
    void aboutRequested();

private:
    Ui::MainToolBar *ui;
    WorkingDirWidget *m_wdirWidget;
    DataSourceHistory *m_dsHistory;

private slots:
    void canGoBackChange(bool can);
    void canGoForwardChange(bool can);
};

#endif // MAINTOOLBAR_H
