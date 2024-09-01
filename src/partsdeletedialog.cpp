#include "partsdeletedialog.h"
#include "ui_partsdeletedialog.h"

#include "partselector.h"

PartsDeleteDialog::PartsDeleteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PartsDeleteDialog)
{
    ui->setupUi(this);

    auto selector = PartSelector::get();
    auto it = selector->allSelectedIterator();

    while (it.hasNext())
    {
        it.next();

        QStringList parts = it.value();

        foreach (const QString &fname, parts)
        {
            ui->partsTextEdit->append(fname);
        }
    }
}

PartsDeleteDialog::~PartsDeleteDialog()
{
    delete ui;
}
