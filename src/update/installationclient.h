#ifndef PARTS_INSTALLATIONCLIENT_H
#define PARTS_INSTALLATIONCLIENT_H
#include <QString>
QString partsInstallationRoot();
QString partsUpdateExecutable();
bool registerPartsInstance(QString *error);
#endif
