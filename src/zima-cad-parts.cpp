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

#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>
#include <QSettings>
#include <QStringList>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(zima_cad_parts);

	QCoreApplication::setOrganizationName("ZIMA-Construction");
	QCoreApplication::setOrganizationDomain("zima-contruction.cz");
	QCoreApplication::setApplicationName("ZIMA-CAD-Parts");

	QApplication a(argc, argv);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

	QTranslator translator;
	QSettings *settings = new QSettings();
	QString lang = settings->value("Language", "detect").toString();

	QString filename = "zima-cad-parts_" + (lang == "detect" ? QLocale::system().name() : lang);
	QStringList paths;

	paths
			<< filename
			<< ("locale/" + filename)
			<< (":/" + filename);

	foreach(QString path, paths)
		if( translator.load(path) )
		{
			a.installTranslator(&translator);
			break;
		}

	delete settings;

	MainWindow w(&translator);
	w.show();

	return a.exec();
}
