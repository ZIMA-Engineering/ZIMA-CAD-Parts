#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QTreeView>

class FileModel;
class FileFilterModel;
class QFileInfo;


class FileView : public QTreeView
{
    Q_OBJECT
public:
    explicit FileView(QWidget *parent = 0);

    QFileInfo fileInfo(const QModelIndex &filteredIndex);
    void createIndexHtmlFile(const QString &text, const QString &fileBase);
    void deleteParts();
    void refreshRequested();

signals:
    void previewProductView(const QFileInfo &fi);
    void hideProductView();

public slots:
    void setDirectory(const QString &path);
    void settingsChanged();
    void copyToWorkingDir();

private:
    QString m_path;
    FileModel *m_model;
    FileFilterModel *m_proxy;

private slots:
    void resizeColumnToContents();
    void refreshModel();
    void handleActivated(const QModelIndex &index);
    void openInProE(const QModelIndex &index);
};

#endif // FILEVIEW_H
