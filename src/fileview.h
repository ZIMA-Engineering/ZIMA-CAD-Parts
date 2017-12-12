#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QTreeView>

class FileViewHeader;
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
	QString currentPath();

signals:
	void previewProductView(const QFileInfo &fi);
	void hideProductView();
	void openPartDirectory(const QFileInfo &fi);

public slots:
	void setDirectory(const QString &path);
	void settingsChanged();
	void copyToWorkingDir();
	void directoryChanged();

protected:
	void scrollContentsBy(int dx, int dy);

private:
	QString m_path;
	FileModel *m_model;
	FileFilterModel *m_proxy;
	FileViewHeader *m_header;

	QModelIndex findNextPartIndex(const QModelIndex &from);

private slots:
	void resizeColumnToContents();
	void refreshModel();
	void handleActivated(const QModelIndex &index);
	void openPart(const QModelIndex &index);
	void showContextMenu(const QPoint &point);
	void editFile();
	void directoryRenamed(const QString &oldName, const QString &newName);
};

#endif // FILEVIEW_H
