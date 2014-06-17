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


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(zima_cad_parts);

	QCoreApplication::setOrganizationName("ZIMA-Construction");
	QCoreApplication::setOrganizationDomain("zima-contruction.cz");
	QCoreApplication::setApplicationName("ZIMA-CAD-Parts");

	QApplication a(argc, argv);
#if 0
    qApp->setStyleSheet("QToolBox::tab {"
                        "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                        "                           stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
                        "                           stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
                        "border: 1px solid black;"
                        "border-radius: 5px;"
                        "xcolor: darkgray;"
                        "}"
                        "QToolBox::tab:selected { /* italicize selected tabs */"
                        "xfont: italic;"
                        "xcolor: white;"
                        "}");
#endif
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

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

	return a.exec();
}
