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

#include <QDialog>
#include <QTranslator>
#include <QSignalMapper>

#include "zima-cad-parts.h"
#include "settings.h"


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

    explicit SettingsDialog(QTranslator **translator, QWidget *parent = 0);
    virtual ~SettingsDialog();

    void setSection(Section s);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private slots:
    void addDataSource();
    void editDataSource();
    void removeDataSource();
    void datasourceUpButton_clicked();
    void datasourceDownButton_clicked();
    void setZimaUtilPath(int util);
    void proeButton_clicked();
    void productViewButton_clicked();
    void textEditorButton_clicked();
    void terminalButton_clicked();

private:
    Ui::SettingsDialog  *m_ui;
    QTranslator **translator;
    QSignalMapper *zimaUtilSignalMapper;
    QList<QLineEdit*> zimaUtilLineEdits;
    DataSourceList m_editedDS;

    void setupDatasourceList();
};

#endif // SETTINGSDIALOG_H
