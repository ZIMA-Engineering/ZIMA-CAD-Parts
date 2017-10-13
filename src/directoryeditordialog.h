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

public slots:
	void apply();

signals:
	void primaryColumnAdded(int row);

private:
	Ui::DirectoryEditorDialog *ui;
	QFileInfo m_fi;
	QString m_dirPath;
	Metadata *m_meta;
	QString m_iconPath;
	QString m_origIconPath;

	void setupLanguageBox();
	void setupIcon();
	void setIcon(const QString &path);
	void installIcon(const QString &icon, const QString &name, bool rename = false);
	void uninstallIcon();
	bool hasAnyLabel();
	bool hasIcon(const QString &name) const;
	QString iconInstallPath(const QString &name) const;
	QStringList findPrimaryLanguageColumns(QStringList languages);

private slots:
	void removeIcon();
	void openIconDialog();
};

#endif // DIRECTORYEDITORDIALOG_H
