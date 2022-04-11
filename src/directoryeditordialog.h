#ifndef DIRECTORYEDITORDIALOG_H
#define DIRECTORYEDITORDIALOG_H

#include <QDialog>
#include <QFileInfo>

class Metadata;

namespace Ui {
class DirectoryEditorDialog;
}

class DirectoryEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DirectoryEditorDialog(const QFileInfo &fi, QWidget *parent = 0);
	~DirectoryEditorDialog();
	QString directoryPath() const;

public slots:
	void apply();

signals:
	void parameterAdded(const QString &handle);
	void parameterHandleChanged(const QString &handle, const QString &newHandle);
	void parameterRemoved(const QString &handle);
	void parametersReordered(const QStringList &parameters);

private:
	Ui::DirectoryEditorDialog *ui;
	QFileInfo m_fi;
	QString m_dirPath;
	Metadata *m_meta;
	QString m_iconPath;
	QString m_origIconPath;
	QStringList m_parameters;
	QHash<QString, QString> m_handleChanges;
	QStringList m_deletedParameters;

	void setupLanguageBox();
	void setupIcon();
	void setIcon(const QString &path);
	void installIcon(const QString &icon, const QString &name, bool rename = false);
	void uninstallIcon();
	bool hasAnyLabel();
	bool hasIcon(const QString &name) const;
	QString iconInstallPath(const QString &name) const;
	int sortOrderToIndex(Qt::SortOrder sortOrder);
	Qt::SortOrder sortOrderFromIndex(int i);

private slots:
	void removeIcon();
	void openIconDialog();
	void parameterAddition(const QString &handle);
	void parameterHandleChange(const QString &handle, const QString &newHandle);
	void parameterRemoval(const QString &handle);
	void reorderParameters(const QStringList &parameters);
};

#endif // DIRECTORYEDITORDIALOG_H
