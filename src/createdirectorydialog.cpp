#include "createdirectorydialog.h"
#include "ui_createdirectorydialog.h"
#include "settings.h"

#include <QDir>
#include <QPushButton>
#include <QDebug>

CreateDirectoryDialog::CreateDirectoryDialog(const QString &path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDirectoryDialog),
    m_path(path)
{
    ui->setupUi(this);

    connect(ui->nameLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(nameChange(QString)));
    nameChange("");

    QDir d(path + "/" + PROTOTYPE_DIR);

    if (!d.exists())
    {
        disablePrototype();
        return;
    }

    auto prototypes = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (prototypes.empty())
    {
        disablePrototype();
        return;
    }

    ui->prototypeComboBox->addItem(tr("No prototype"));

    foreach (const QString &p, prototypes)
        ui->prototypeComboBox->addItem(p, p);
}

CreateDirectoryDialog::~CreateDirectoryDialog()
{
    delete ui;
}

QString CreateDirectoryDialog::name() const
{
    return ui->nameLineEdit->text();
}

bool CreateDirectoryDialog::hasPrototype() const
{
    return ui->prototypeComboBox->count() > 1 && ui->prototypeComboBox->currentIndex() > 0;
}

QString CreateDirectoryDialog::prototype() const
{
    return ui->prototypeComboBox->currentData().toString();
}

void CreateDirectoryDialog::disablePrototype()
{
    ui->prototypeComboBox->addItem(tr("No prototype available"));
    ui->prototypeComboBox->setDisabled(true);
}

void CreateDirectoryDialog::nameChange(const QString &text)
{
    bool ok = !text.isEmpty() && !QFile::exists(m_path + "/" + text);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
}
