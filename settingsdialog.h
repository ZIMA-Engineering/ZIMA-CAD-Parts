#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QSettings>
#include <QVector>
#include "ftpserver.h"

class QListWidgetItem;

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(SettingsDialog)
public:
    explicit SettingsDialog(QWidget *parent = 0);
    virtual ~SettingsDialog();

    void loadSettings(QSettings*);
    void saveSettings(QSettings*);

    QVector<FtpServer*> *getData() { return &servers; }

protected:
    virtual void changeEvent(QEvent *e);
    void updateServerList();

private slots:
    void addFtpServer();
    void removeFtpServer();
    void selectFtpServer(QListWidgetItem*, QListWidgetItem*);
    void changingText();
    void changingPassive();

private:
    Ui::SettingsDialog  *m_ui;
    QVector<FtpServer*>   servers;
    FtpServer           *currentServer;
};

#endif // SETTINGSDIALOG_H
