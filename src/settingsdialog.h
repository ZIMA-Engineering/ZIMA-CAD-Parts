/*
  ZIMA-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QSettings>
#include <QMultiHash>
#include <QVector>
#include <QTranslator>
#include "zima-parts.h"
#include "basedatasource.h"
#include "ftpdatasource.h"
#include "localdatasource.h"

#ifdef INCLUDE_PRODUCT_VIEW
#include "extensions/productview/productviewsettings.h"
#endif // INCLUDE_PRODUCT_VIEW

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
	void pruneCache(QString path = QString());
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
#ifdef INCLUDE_PRODUCT_VIEW
	ProductViewSettings *productViewSettings;
#endif // INCLUDE_PRODUCT_VIEW
};

#endif // SETTINGSDIALOG_H
