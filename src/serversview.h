#ifndef SERVERSVIEW_H
#define SERVERSVIEW_H

#include <QTreeView>
#warning todo make settings dialog calling simpler
#include "settingsdialog.h"

class ServersModel;
class ServersProxyModel;
class QSignalMapper;
class QFileInfo;


/*! Main class for "directory tree" widget. Every datasource has one
 * ServersView.
 */
class ServersView : public QTreeView
{
    Q_OBJECT
public:
    explicit ServersView(const QString &rootPath, QWidget *parent = 0);

    /*! try to find the path.
     * @returns true if the path can be found
     */
    bool navigateToDirectory(const QString &path);

signals:
    void showSettings(SettingsDialog::Section);
    void workingDirChanged();
    void directorySelected(const QString &path);

private:
    QString m_path;
    ServersModel *m_model;
    ServersProxyModel *m_proxy;
    QSignalMapper *m_signalMapper;

    QFileInfo currentFileInfo();

private slots:
    void refreshModel();
    void modelClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &point);
    void spawnZimaUtilityOnDir(const QString &label);
    void indexOpenPath();
    void setWorkingDirectory();
};

#endif // SERVERSVIEW_H
