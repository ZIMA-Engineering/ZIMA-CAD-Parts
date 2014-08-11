#ifndef SERVERSWIDGET_H
#define SERVERSWIDGET_H

#include "ui_serverswidget.h"
#include "settingsdialog.h"
#include "servertabwidget.h"

class QSignalMapper;


/*!
 * \brief The data source tree and ServerTabWidget user interface.
 *
 * The data source tree is kept in NavBar
 */
class ServersWidget : public QWidget, public Ui::ServersWidget
{
	Q_OBJECT
public:
	explicit ServersWidget(QWidget *parent = 0);

	QModelIndex currentIndex();
    void setDirectory(const QString &path);

signals:
	void statusUpdated(const QString &message);
	void showSettings(SettingsDialog::Section);

	void itemLoaded(const QModelIndex&);
	void clicked(const QModelIndex&);
	void activated(const QModelIndex&);

	void techSpecAvailable(const QUrl&);

	void workingDirChanged();
    void directorySelected(const QString&);

public slots:
	void expand(const QModelIndex & index);
	void settingsChanged();
	void goToWorkingDirectory();

private:
	QStringList m_zimaUtils;

private slots:
	void splitterMoved(int, int);
};

#endif // SERVERSWIDGET_H
