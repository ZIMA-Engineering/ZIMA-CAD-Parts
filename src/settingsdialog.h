/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

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
#include <QTranslator>
#include <QSignalMapper>
#include <QList>
#include <QLineEdit>

#include "zima-cad-parts.h"
#include "basedatasource.h"
#include "ftpdatasource.h"
#include "localdatasource.h"

#include "extensions/productview/productviewsettings.h"

class QListWidgetItem;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
	Q_OBJECT
	Q_DISABLE_COPY(SettingsDialog)
public:
	enum Section {
	    General=0,
	    DataSources,
	    ExternalPrograms,
	    DeveloperMode,
	    ProductView,
	    SectionCount
	};

	enum Languages {
	    DETECT=0,
	    ENGLISH,
	    CZECH
	};

	explicit SettingsDialog(QSettings *settings, QList<BaseDataSource*> datasources, QTranslator **translator, QWidget *parent = 0);
	virtual ~SettingsDialog();

	void setSection(Section s);
	void loadSettings(QSettings*);
	void saveSettings();

	QList<BaseDataSource*> getDatasources();
	static QString langIndexToName(int lang);
	static int langIndex(QString lang);

protected:
	virtual void changeEvent(QEvent *e);

private slots:
	void addDataSource();
	void editDataSource();
	void removeDataSource();
	void datasourceUpButton_clicked();
	void datasourceDownButton_clicked();
	void pruneCache(QString path = QString());
//	void changingText();
//	void changingPassive();
	void setZimaUtilPath(int util);

private:
	Ui::SettingsDialog  *m_ui;
	QSettings *settings;
	QTranslator **translator;
	QSignalMapper *zimaUtilSignalMapper;
	QList<QLineEdit*> zimaUtilLineEdits;
	ProductViewSettings *productViewSettings;

	void setupDatasourceList(QList<BaseDataSource*> datasources);
};

#endif // SETTINGSDIALOG_H
