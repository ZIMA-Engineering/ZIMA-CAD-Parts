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
#include "mainwindow.h"
#include "settings.h"

/**
\mainpage ZIMA-CAD-Parts Developer Documentation

ZIMA-CAD-Parts is a tool, develop by ZIMA-Engineering (www.zima-engineering.cz),
used by design engineers for management of CAD files.

It can manage local CAD projects with the possibility of connection with PDM system.
It is also a source of library CAD parts. It works with local or remote data storage.
It supports Pro/Engineer, CATIA, NX (UGS), SolidWorks, SolidEdge, Inventor and
neutral formats, i.e. STEP, IGES, DWG, DXF, STL, BLEND and PDF.

The application is written in C++/Qt, it is multiplatform and supports Windows, MAC and Linux.
*/

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(zima_cad_parts);

	QCoreApplication::setOrganizationName("ZIMA-Construction");
	QCoreApplication::setOrganizationDomain("zima-contruction.cz");
	QCoreApplication::setApplicationName("ZIMA-CAD-Parts");

	QApplication a(argc, argv);

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
#endif

	QTranslator translator;
	QString lang = Settings::get()->getCurrentLanguageCode();

	QString filename = "zima-cad-parts_" + lang;
	QStringList paths;

	paths
	        << filename
	        << ("locale/" + filename)
	        << (":/" + filename);

#ifdef Q_OS_MAC
	paths << QCoreApplication::applicationDirPath() + "/../Resources/" + filename;
#endif

	foreach(QString path, paths)
	if( translator.load(path) )
	{
		a.installTranslator(&translator);
		break;
	}

	MainWindow w(&translator);
	w.show();

	int ret = a.exec();

	Settings::get()->save();
	return ret;
}
