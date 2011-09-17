#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModelIndex>
#include <QLabel>
#include "settingsdialog.h"

namespace Ui
{
    class MainWindowClass;
}

class QFtp;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindowClass *ui;
    SettingsDialog      *settingsDlg;
    QSettings           *settings;
    QVector<FtpServer*> *servers;
    QLabel              *statusState, *statusDir;
    FtpServer           *currentServer;

    void closeEvent(QCloseEvent*);

public slots:
    void downloadButton();
    void setWorkingDirectory();
    void showSettings();
    void updateClicked();
    void searchClicked();
    void serverSelected(const QModelIndex&);
    void updateStatus(QString);
    void treeExpandedOrCollaped(const QModelIndex&);
    void serverLoaded();
    void itemLoaded(const QModelIndex&);
    void convertModelIndex(const QModelIndex&);
    void loadTechSpec(QUrl url);
    void errorOccured(QString error);
    void filesDownloaded();
};

#endif // MAINWINDOW_H
