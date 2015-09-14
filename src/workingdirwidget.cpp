#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>

#include "workingdirwidget.h"
#include "settings.h"
#include "ui_workingdirwidget.h"


WorkingDirWidget::WorkingDirWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::WorkingDirWidget)
{
	ui->setupUi(this);

	connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(setWorkingDirectoryDialog()));
	connect(ui->openWorkDirButton, SIGNAL(clicked()), this, SLOT(openWorkingDirectory()));
}

WorkingDirWidget::~WorkingDirWidget()
{
	delete ui;
}

void WorkingDirWidget::settingsChanged()
{
    ui->workingDirectoryEdit->setText(Settings::get()->getWorkingDir());
}

void WorkingDirWidget::setWorkingDirectoryDialog()
{
	QString str = QFileDialog::getExistingDirectory(this,
	              tr("ZIMA-CAD-Parts - set working directory"),
                  Settings::get()->getWorkingDir());
	if (!str.isEmpty())
	{
        Settings::get()->setWorkingDir(str);
		settingsChanged();
	}
}

void WorkingDirWidget::openWorkingDirectory()
{
    if(!QFile::exists(Settings::get()->getWorkingDir()))
	{
		QDir dir;

        if(!dir.mkpath(Settings::get()->getWorkingDir()))
		{
			QMessageBox::warning(this, tr("Unable to create working directory"),
                                 tr("Unable to create working directory: %1").arg(Settings::get()->getWorkingDir()));
			return;
		}
	}

    QDesktopServices::openUrl(QUrl::fromLocalFile(Settings::get()->getWorkingDir()));
}
