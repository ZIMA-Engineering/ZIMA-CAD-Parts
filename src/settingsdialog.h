#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QSettings>
#include <QMultiHash>
#include <QVector>
#include <QTranslator>
#include "basedatasource.h"
#include "ftpdatasource.h"
#include "localdatasource.h"

class QListWidgetItem;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
	Q_OBJECT
	Q_DISABLE_COPY(SettingsDialog)
public:
	enum Languages {
		DETECT=0,
		ENGLISH,
		CZECH
	};

	explicit SettingsDialog(QSettings *settings, QVector<BaseDataSource*> servers, QTranslator **translator, QWidget *parent = 0);
	virtual ~SettingsDialog();

	void loadSettings(QSettings*);
	void saveSettings();

	QVector<BaseDataSource*> getData();

protected:
	virtual void changeEvent(QEvent *e);
	void updateServerList();
	int langIndex(QString lang);
	QString langIndexToName(int lang);

private slots:
	void addDataSource();
	void editDataSource();
	void removeDataSource();
	void selectDataSource(QListWidgetItem*, QListWidgetItem*);
//	void changingText();
//	void changingPassive();

private:
	Ui::SettingsDialog  *m_ui;
	QVector<BaseDataSource*> originServers;
	QVector<BaseDataSource*> servers;
	//QHash<QListWidgetItem*, BaseDataSource*> lastUsedDataSources;
	BaseDataSource           *currentServer;
	QSettings *settings;
	QTranslator **translator;
};

#endif // SETTINGSDIALOG_H
