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
	Q_INIT_RESOURCE(zima_parts);

	QCoreApplication::setOrganizationName("ZIMA-Construction");
	QCoreApplication::setOrganizationDomain("zima-contruction.cz");
	QCoreApplication::setApplicationName("ZIMA-Parts");

	QApplication a(argc, argv);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));

	QTranslator translator;
	QSettings *settings = new QSettings();
	QString lang = settings->value("Language", "detect").toString();

	QString filename = "zima-parts_" + (lang == "detect" ? QLocale::system().name() : lang);
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
