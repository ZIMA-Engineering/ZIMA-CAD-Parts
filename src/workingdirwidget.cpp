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
}

WorkingDirWidget::~WorkingDirWidget()
{
    delete ui;
}

void WorkingDirWidget::settingsChanged()
{
    ui->workingDirectoryEdit->setText(Settings::get()->getWorkingDir());
}
