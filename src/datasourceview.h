#ifndef DATASOURCEVIEW_H
#define DATASOURCEVIEW_H

#include <QTreeView>
#include <QStringList>

#include "settingsdialog.h"

class DataSourceModel;
class DataSourceProxyModel;
class ScriptRunner;
class QSignalMapper;
class QFileInfo;

/*! Main class for "directory tree" widget. Every datasource has one
 * ServersView.
 */
class DataSourceView : public QTreeView
{
    Q_OBJECT
public:
    explicit DataSourceView(const QString &rootPath, QWidget *parent = 0);

    /*! try to find the path.
     * @returns true if the path can be found
     */
    bool navigateToDirectory(const QString &path);

public slots:
    void releaseFileSystemModel();
    void restoreFileSystemModelState();

signals:
    void showSettings(SettingsDialog::Section);
    void workingDirChanged();
    void directorySelected(const QString &path);
    void directoryChanged(const QString &path);
    void openInANewTabRequested(const QString &path);

private:
    QString m_path;
    DataSourceModel *m_model;
    DataSourceProxyModel *m_proxy;
    QSignalMapper *m_signalMapper;
    ScriptRunner *m_scriptRunner;
    QStringList m_expandedPathsBeforeReset;
    QString m_currentPathBeforeReset;
    int m_verticalScrollBeforeReset;
    int m_horizontalScrollBeforeReset;
    bool m_hasModelState;

    QFileInfo currentFileInfo();
    void addScriptsToContextMenu(QMenu *menu);
    void setupModel();
    void saveFileSystemModelState();
    void collectExpandedPaths(const QModelIndex &parent);
    bool expandPath(const QString &path);
    bool pathBelongsToRoot(const QString &path) const;
    QString nearestExistingPath(const QString &path) const;

private slots:
    void refreshModel();
    void modelClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &point);
    void spawnZimaUtilityOnDir(const QString &label);
    void indexOpenPath();
    void openInANewTab();
    void setWorkingDirectory();
    void createDirectory();
    void editDirectory();
    void copyDirectoryAs();
    void deleteDirectory();
    void runScriptOnDir(const QFileInfo &dir, const QFileInfo &script);
};

#endif // DATASOURCEVIEW_H
