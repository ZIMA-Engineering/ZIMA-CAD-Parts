#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(zimaparts);

	QCoreApplication::setOrganizationName("ZIMA-Construction");
	QCoreApplication::setOrganizationDomain("zima-contruction.cz");
	QCoreApplication::setApplicationName("ZIMA-Parts");

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
