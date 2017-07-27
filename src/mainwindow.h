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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QLabel>
#include <QUrl>
#include <QThread>
#include <QTranslator>
#include <QButtonGroup>
#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QSignalMapper>

#include "settingsdialog.h"
#include "filemodel.h"
#include "zima-cad-parts.h"
#include "filefiltermodel.h"

class WorkingDirWidget;

namespace Ui
{
class MainWindowClass;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QTranslator *translator, QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindowClass *ui;

	QTranslator *translator;// app ui
	WorkingDirWidget *m_wdirWidget;

	// History
	QList<QString> m_history;
	int m_historyCurrent;

	QModelIndex lastFoundIndex;

	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent*);

	void handleHistory();

public slots:
	void showSettings(SettingsDialog::Section section = SettingsDialog::General);

private slots:
	void settingsChanged();
	void trackHistory(const QString &index);
	void historyBack();
	void historyForward();
};

class SleeperThread : public QThread
{
public:
	static void msleep(unsigned long msecs)
	{
		QThread::msleep(msecs);
	}
};

#endif // MAINWINDOW_H
