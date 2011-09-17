#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(zimaparts);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
