/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "settingsdialog.h"
#include "updatespage.h"
#include "settings.h"
#include "ui_settingsdialog.h"

#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QLocale>
#include <QTimer>
#include <QToolButton>

#include "addeditdatasource.h"
#include "savedpasswordsdialog.h"
#include <QCheckBox>
#include "core/partstools.h"

//! A helpter template class to convert any pointer to QVariant and vice versa
template <class T> class PtrVariant
{
public:
    static T* asPtr(QVariant v)
    {
        return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
        return QVariant::fromValue((void *) ptr);
    }
};

// store datasource in qlistwidgetitem to save additional qlist handling
#define DATASOURCE_ROLE Qt::UserRole+1
// role to sign if the datasource is used in app or just created here
//    true = created here; false = used in app (do not delete)
#define UNUSED_ROLE Qt::UserRole+2


SettingsDialog::SettingsDialog(QTranslator **translator, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingsDialog),
    translator(translator)
{
    m_ui->setupUi(this);
    m_updatesPage = new UpdatesPage(this);
    m_ui->tabWidget->addTab(m_updatesPage, tr("Updates"));

    connect(m_ui->btnAdd, SIGNAL(clicked()), this, SLOT(addDataSource()));
    connect(m_ui->editBtn, SIGNAL(clicked()), this, SLOT(editDataSource()));
    connect(m_ui->btnRemove, SIGNAL(clicked()), this, SLOT(removeDataSource()));
    connect(m_ui->datasourceUpButton, SIGNAL(clicked()),
            this, SLOT(datasourceUpButton_clicked()));
    connect(m_ui->datasourceDownButton, SIGNAL(clicked()),
            this, SLOT(datasourceDownButton_clicked()));
    connect(m_ui->textEditorButton, SIGNAL(clicked()),
            this, SLOT(textEditorButton_clicked()));
    connect(m_ui->terminalButton, SIGNAL(clicked()),
            this, SLOT(terminalButton_clicked()));
    connect(m_ui->savedPasswordsButton, SIGNAL(clicked()),
            this, SLOT(openSavedPasswordsDialog()));

    m_editedDS = Settings::get()->DataSources;
    setupDatasourceList();

    m_ui->spinPicture->setValue(Settings::get()->GUIThumbWidth);
    m_ui->previewWidthSpinBox->setValue(Settings::get()->GUIPreviewWidth);
    m_ui->languageComboBox->setCurrentIndex(Settings::get()->langIndex(Settings::get()->languagePreference()));
    m_ui->splashGroupBox->setChecked(Settings::get()->GUISplashEnabled);
    m_ui->splashDurationSpinBox->setValue(Settings::get()->GUISplashDuration);
    m_ui->techSpecToolBarCheckBox->setChecked(Settings::get()->TechSpecToolBarEnabled);
    m_ui->browserSavePasswordsCheckBox->setChecked(Settings::get()->BrowserSaveFormPasswords);
    m_ui->browserAutofillCheckBox->setChecked(Settings::get()->BrowserAutoFillPasswords);
    m_ui->browserRememberHttpAuthCheckBox->setChecked(Settings::get()->BrowserRememberHttpAuth);
    m_ui->textEditorLineEdit->setText(Settings::get()->TextEditorPath);
    m_ui->terminalLineEdit->setText(Settings::get()->TerminalPath);

    connect(m_ui->proeButton, SIGNAL(clicked()),
            this, SLOT(proeButton_clicked()));

    m_toolsRecursive = new QCheckBox(tr("Include subdirectories by default"), this);
    m_toolsRecursive->setChecked(Settings::get()->ToolsRecursive);
    m_toolsCleanOld = new QCheckBox(tr("Cleaner: remove old numbered versions"), this);
    m_toolsCleanOld->setChecked(Settings::get()->ToolsCleanOld);
    m_toolsMasks = new QLineEdit(Settings::get()->ToolsCleanMasks.join(';'), this);
    m_ui->gridLayout->addWidget(m_toolsRecursive, 0, 0, 1, 2);
    m_ui->gridLayout->addWidget(m_toolsCleanOld, 1, 0, 1, 2);
    m_ui->gridLayout->addWidget(new QLabel(tr("Cleaner: additional masks (semicolon separated)"), this), 2, 0);
    m_ui->gridLayout->addWidget(m_toolsMasks, 2, 1);
    auto runtime = new QLineEdit(PartsCore::ghostscriptExecutable(), this);
    runtime->setReadOnly(true);
    runtime->setPlaceholderText(tr("Ghostscript runtime is missing"));
    m_ui->gridLayout->addWidget(new QLabel("Ghostscript", this), 3, 0);
    m_ui->gridLayout->addWidget(runtime, 3, 1);

    m_ui->proeEdit->setText(Settings::get()->ProeExecutable);

}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        m_ui->tabWidget->setTabText(Updates, tr("Updates"));
        break;
    default:
        break;
    }
}

void SettingsDialog::setSection(SettingsDialog::Section s)
{
    m_ui->tabWidget->setCurrentIndex(s);
}

void SettingsDialog::setCurrentDataSource(DataSource *dataSource)
{
    if (!dataSource)
        return;

    setSection(SettingsDialog::DataSources);

    for (int row = 0; row < m_ui->datasourceList->count(); ++row)
    {
        QListWidgetItem *item = m_ui->datasourceList->item(row);
        DataSource *itemDataSource = PtrVariant<DataSource>::asPtr(item->data(DATASOURCE_ROLE));

        if (itemDataSource == dataSource
                || (itemDataSource
                    && itemDataSource->name == dataSource->name
                    && itemDataSource->rootPath == dataSource->rootPath))
        {
            m_ui->datasourceList->setCurrentRow(row);
            m_ui->datasourceList->setFocus();
            return;
        }
    }
}

void SettingsDialog::openCurrentDataSourceEditor()
{
    QTimer::singleShot(0, this, SLOT(editDataSource()));
}

void SettingsDialog::accept()
{
    m_updatesPage->save();
    Settings::get()->GUIThumbWidth = m_ui->spinPicture->value();
    Settings::get()->GUIPreviewWidth = m_ui->previewWidthSpinBox->value();
    Settings::get()->GUISplashEnabled = m_ui->splashGroupBox->isChecked();
    Settings::get()->GUISplashDuration = m_ui->splashDurationSpinBox->value();
    Settings::get()->TechSpecToolBarEnabled = m_ui->techSpecToolBarCheckBox->isChecked();
    Settings::get()->BrowserSaveFormPasswords = m_ui->browserSavePasswordsCheckBox->isChecked();
    Settings::get()->BrowserAutoFillPasswords = m_ui->browserAutofillCheckBox->isChecked();
    Settings::get()->BrowserRememberHttpAuth = m_ui->browserRememberHttpAuthCheckBox->isChecked();
    Settings::get()->TextEditorPath = m_ui->textEditorLineEdit->text();
    Settings::get()->TerminalPath = m_ui->terminalLineEdit->text();


    QString lang = Settings::get()->langIndexToName( m_ui->languageComboBox->currentIndex() );
    Settings::get()->setCurrentLanguageCode(lang);
    MetadataCache::get()->clear();

    Settings::get()->ToolsRecursive = m_toolsRecursive->isChecked();
    Settings::get()->ToolsCleanOld = m_toolsCleanOld->isChecked();
    Settings::get()->ToolsCleanMasks.clear();
    for (const auto &mask : m_toolsMasks->text().split(';', Qt::SkipEmptyParts))
        if (!mask.trimmed().isEmpty()) Settings::get()->ToolsCleanMasks.append(mask.trimmed());

    Settings::get()->ProeExecutable = m_ui->proeEdit->text();

    if (m_editedDS != Settings::get()->DataSources)
    {
        // Note: do not delete datasources here. It will be handled in ServersWidget::settingsChanged()
        Settings::get()->DataSources.clear();
        Settings::get()->DataSources = m_editedDS;
        Settings::get()->DataSourcesNeedsUpdate = true;
    }

    QDialog::accept();
}

void SettingsDialog::addDataSource()
{
    AddEditDataSource addEdit(m_editedDS, 0, AddEditDataSource::ADD);

    if (addEdit.exec() == QDialog::Accepted )
    {
        DataSource *ds = addEdit.dataSource();
        QListWidgetItem *item =  new QListWidgetItem(ds->icon, ds->name);
        item->setData(DATASOURCE_ROLE, PtrVariant<DataSource>::asQVariant(ds));
        item->setData(UNUSED_ROLE, true);
        m_ui->datasourceList->addItem(item);
        m_ui->datasourceList->setCurrentItem(item);
        m_editedDS.append(ds);
    }
}

void SettingsDialog::editDataSource()
{
    if ( !m_ui->datasourceList->count() )
        return;

    QListWidgetItem *item = m_ui->datasourceList->currentItem();
    if (!item)
        return;

    DataSource *ds = PtrVariant<DataSource>::asPtr(item->data(DATASOURCE_ROLE));
    AddEditDataSource addEdit(m_editedDS, ds, AddEditDataSource::EDIT);

    if (addEdit.exec())
    {
        m_editedDS.removeAll(ds);
        ds = addEdit.dataSource();

        item->setData(DATASOURCE_ROLE, PtrVariant<DataSource>::asQVariant(ds));
        item->setIcon(ds->icon);
        item->setText(ds->name);
        m_editedDS.append(ds);
        Settings::get()->DataSourcesNeedsUpdate = true;
    }
}

void SettingsDialog::removeDataSource()
{
    if ( !m_ui->datasourceList->count() )
        return;

    QListWidgetItem *it = m_ui->datasourceList->currentItem();
    if (!it)
        return;

    DataSource *ds = PtrVariant<DataSource>::asPtr(it->data(DATASOURCE_ROLE));
    if (it->data(UNUSED_ROLE).toBool())
    {
        m_editedDS.removeAll(ds);
        delete ds;
    }

    int row = m_ui->datasourceList->currentRow();
    m_ui->datasourceList->takeItem(row);
    delete m_editedDS.takeAt(row);

    delete it;
}

void SettingsDialog::datasourceUpButton_clicked()
{
    QListWidget *lw = m_ui->datasourceList;
    QListWidgetItem *current = lw->currentItem();
    int ix = lw->row(current);
    if (ix > 0)
    {
        QListWidgetItem *temp = lw->takeItem(ix);
        lw->insertItem(ix-1, temp);
        lw->setCurrentRow(ix-1);
        m_editedDS.swapItemsAt(ix, ix-1);
    }
}

void SettingsDialog::datasourceDownButton_clicked()
{
    QListWidget *lw = m_ui->datasourceList;
    QListWidgetItem *current = lw->currentItem();
    int ix = lw->row(current);
    if (ix < lw->count()-1)
    {
        QListWidgetItem *temp = lw->takeItem(ix);
        lw->insertItem(ix+1, temp);
        lw->setCurrentRow(ix+1);
        m_editedDS.swapItemsAt(ix, ix+1);
    }
}

void SettingsDialog::openSavedPasswordsDialog()
{
    SavedPasswordsDialog dialog(this);
    dialog.exec();
}

void SettingsDialog::setupDatasourceList()
{
    m_ui->datasourceList->clear();
    if (!m_editedDS.count())
        return;

    foreach(DataSource *s, m_editedDS)
    {
        QListWidgetItem *i = new QListWidgetItem(s->icon, s->name);
        i->setData(DATASOURCE_ROLE, PtrVariant<DataSource>::asQVariant(s));
        i->setData(UNUSED_ROLE, false);
        m_ui->datasourceList->addItem(i);
    }

    m_ui->datasourceList->setCurrentRow(0);
    m_ui->datasourceList->setFocus();
}


void SettingsDialog::proeButton_clicked()
{
    QString exe = QFileDialog::getOpenFileName(this, tr("Locate ProE launcher"),
                  QDir::currentPath(),
                  tr("ProE executable (proe.exe);;All files (*)"));
    if (exe.isNull())
        return;
    m_ui->proeEdit->setText(exe);
}

void SettingsDialog::textEditorButton_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, tr("ZIMA-CAD-Parts - select text editor"),
                  QDir::currentPath());
    if (!str.isEmpty())
        m_ui->textEditorLineEdit->setText(str);
}

void SettingsDialog::terminalButton_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, tr("ZIMA-CAD-Parts - select terminal"),
                  QDir::currentPath());
    if (!str.isEmpty())
        m_ui->terminalLineEdit->setText(str);
}
