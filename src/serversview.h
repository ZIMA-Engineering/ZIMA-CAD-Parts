#ifndef SERVERSVIEW_H
#define SERVERSVIEW_H

#include <QTreeView>
#warning todo make settings dialog calling simpler
#include "settingsdialog.h"

class ServersModel;
class ServersProxyModel;
class QSignalMapper;
class QFileInfo;


class ServersView : public QTreeView
{
    Q_OBJECT
public:
    explicit ServersView(const QString &rootPath, QWidget *parent = 0);

    bool navigateToDirectory(const QString &path);

signals:
    void showSettings(SettingsDialog::Section);
    void workingDirChanged();
    void directorySelected(const QString &path);

public slots:

private:
    ServersModel *m_model;
    ServersProxyModel *m_proxy;
    QSignalMapper *m_signalMapper;

    QFileInfo currentFileInfo();

private slots:
    void modelClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &point);
    void spawnZimaUtilityOnDir(const QString &label);
    void indexOpenPath();
    void setWorkingDirectory();
};

#endif // SERVERSVIEW_H
