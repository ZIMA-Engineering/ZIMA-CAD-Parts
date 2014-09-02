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
};

#endif // FILEVIEW_H
