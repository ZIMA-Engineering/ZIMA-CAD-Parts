#include "proeproductview.h"
#include "../../settings.h"
#include "../../zima-cad-parts.h"

#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>


ProEProductView::ProEProductView(QWidget *parent) :
	AbstractProductView(parent),
	ui(new Ui::ProEProductView())
{
	ui->setupUi(this);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavaEnabled, true);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
}

ProEProductView::~ProEProductView()
{
	delete ui;
}

QString ProEProductView::title()
{
	return tr("PRO/E part");
}

QList<File::FileType> ProEProductView::canHandle()
{
#ifdef Q_OS_WIN
	// ProductView Express is available for windows only
	return QList<File::FileTypes>() << File::PRT_PROE << File::PRT_NX;
#else
	return QList<File::FileType>();
#endif
}

bool ProEProductView::handle(FileMetadata *f)
{
	QFile pv(":/data/extensions/productview/proeproductview.html");
	pv.open(QIODevice::ReadOnly);
	QTextStream stream(&pv);
	QString html = stream.readAll();

	html.replace("%VERSION%", VERSION);
    html.replace("%FILE_NAME%", f->fileInfo.fileName());
    html.replace("%FILE_PATH%", f->fileInfo.filePath());
	html.replace("%PRODUCTVIEW_PATH%", Settings::get()->ExtensionsProductViewPath);

    QTemporaryFile tmp(QDir::tempPath() + "/zima-cad-parts_XXXXXX_" + f->fileInfo.fileName() + ".html");

	if(tmp.open())
	{
		QTextStream out(&tmp);
		out << html;
		out.flush();

		// It doesn't work with double slash in windows
		ui->appletWebView->setUrl(QUrl(QString("file:/%1").arg(tmp.fileName())));

		//tmp.setAutoRemove(false);
		tmp.close();
	} else {
		ui->appletWebView->setHtml("Sorry, cannot create temporary file.");
		return false;
	}

	return true;
}
