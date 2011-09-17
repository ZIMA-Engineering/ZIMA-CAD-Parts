#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <QString>
#include "ftpdata.h"

class QListWidgetItem;
class FtpData;

struct FtpServer
{
	FtpServer();

	QListWidgetItem *lwItem;
	QString address;
	int     port;
	bool    passiveMode;
	QString login;
	QString password;
	QString baseDir;

	FtpData* ftpData;
};

#endif // FTPSERVER_H
