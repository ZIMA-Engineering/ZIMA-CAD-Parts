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
#include "settings.h"
#include "ui_settingsdialog.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QLocale>
#include <QToolButton>

#include "addeditdatasource.h"
#include "baseremotedatasource.h"
#include "zimautils.h"

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
		return qVariantFromValue((void *) ptr);
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

	connect(m_ui->btnAdd, SIGNAL(clicked()), this, SLOT(addDataSource()));
	connect(m_ui->editBtn, SIGNAL(clicked()), this, SLOT(editDataSource()));
	connect(m_ui->btnRemove, SIGNAL(clicked()), this, SLOT(removeDataSource()));
	connect(m_ui->datasourceUpButton, SIGNAL(clicked()),
	        this, SLOT(datasourceUpButton_clicked()));
	connect(m_ui->datasourceDownButton, SIGNAL(clicked()),
	        this, SLOT(datasourceDownButton_clicked()));
	connect(m_ui->pruneCacheButton, SIGNAL(clicked()), this, SLOT(pruneCache()));
	connect(m_ui->productViewButton, SIGNAL(clicked()),
	        this, SLOT(productViewButton_clicked()));

    setupDatasourceList();

    m_ui->spinPicture->setValue(Settings::get()->GUIThumbWidth);
    m_ui->previewWidthSpinBox->setValue(Settings::get()->GUIPreviewWidth);
    m_ui->languageComboBox->setCurrentIndex(Settings::get()->langIndex(Settings::get()->getCurrentLanguageCode()));
    m_ui->splashGroupBox->setChecked(Settings::get()->GUISplashEnabled);
    m_ui->splashDurationSpinBox->setValue(Settings::get()->GUISplashDuration);
    m_ui->developerModeGroupBox->setChecked(Settings::get()->DeveloperEnabled);
    m_ui->techSpecToolBarCheckBox->setChecked(Settings::get()->DeveloperTechSpecToolBar);
    m_ui->productViewEdit->setText(Settings::get()->ExtensionsProductViewPath);

	connect(m_ui->proeButton, SIGNAL(clicked()),
	        this, SLOT(proeButton_clicked()));

	zimaUtilSignalMapper = new QSignalMapper(this);

	connect(zimaUtilSignalMapper, SIGNAL(mapped(int)), this, SLOT(setZimaUtilPath(int)));

    QHashIterator<QString,QString> it(Settings::get()->ExternalPrograms);
    int i = 0;
    while (it.hasNext())
    {
        it.next();

		QToolButton *t = new QToolButton(this);
		t->setText("...");

		connect(t, SIGNAL(clicked()), zimaUtilSignalMapper, SLOT(map()));

        zimaUtilLineEdits << new QLineEdit(it.value(), this);

        m_ui->gridLayout->addWidget(new QLabel(it.key(), this), i, 0);
		m_ui->gridLayout->addWidget(zimaUtilLineEdits.last(), i, 1);
		m_ui->gridLayout->addWidget(t, i, 2);

		zimaUtilSignalMapper->setMapping(t, i);
        ++i;
	}

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
		break;
	default:
		break;
	}
}

void SettingsDialog::setSection(SettingsDialog::Section s)
{
	m_ui->tabWidget->setCurrentIndex(s);
}

void SettingsDialog::accept()
{
    Settings::get()->GUIThumbWidth = m_ui->spinPicture->value();
    Settings::get()->GUIPreviewWidth = m_ui->previewWidthSpinBox->value();
    Settings::get()->GUISplashEnabled = m_ui->splashGroupBox->isChecked();
    Settings::get()->GUISplashDuration = m_ui->splashDurationSpinBox->value();
    Settings::get()->DeveloperEnabled = m_ui->developerModeGroupBox->isChecked();
    Settings::get()->DeveloperTechSpecToolBar = m_ui->techSpecToolBarCheckBox->isChecked();
    Settings::get()->ExtensionsProductViewPath = m_ui->productViewEdit->text();


    QString lang = Settings::get()->langIndexToName( m_ui->languageComboBox->currentIndex() );
    if (lang != Settings::get()->getCurrentLanguageCode())
	{
		qApp->removeTranslator(*translator);

		QTranslator *t = new QTranslator(parent());
		QString filename = "zima-cad-parts_" + (lang == "detect" ? QLocale::system().name() : lang);
		QStringList paths;

		paths
		        << filename
		        << ("locale/" + filename)
		        << (":/" + filename);

#ifdef Q_OS_MAC
		paths << QCoreApplication::applicationDirPath() + "/../Resources/" + filename;
#endif

		foreach(QString path, paths)
		if( t->load(path) )
		{
			qApp->installTranslator(t);
			*translator = t;
			break;
		}
	}

    Settings::get()->setCurrentLanguageCode(lang);

    QHashIterator<QString,QString> it(Settings::get()->ExternalPrograms);
    int i = 0;
    while (it.hasNext())
    {
        it.next();
        Settings::get()->ExternalPrograms[it.key()] = zimaUtilLineEdits[i]->text();
        ++i;
	}

    Settings::get()->ProeExecutable = m_ui->proeEdit->text();

    // datasources
    DataSourceList newDSList;
    for (int i = 0; i < m_ui->datasourceList->count(); ++i)
    {
        QListWidgetItem *item = m_ui->datasourceList->item(i);
        newDSList << PtrVariant<BaseDataSource>::asPtr(item->data(DATASOURCE_ROLE));
    }

    if (newDSList != Settings::get()->DataSources)
    {
        // Note: do not delete datasources here. It will be handled in ServersWidget::settingsChanged()
        //qDeleteAll(Settings::get()->DataSources);
        Settings::get()->DataSources.clear();
        Settings::get()->DataSources = newDSList;
        Settings::get()->DataSourcesNeedsUpdate = true;
    }

    QDialog::accept();
}

void SettingsDialog::addDataSource()
{
	BaseDataSource *dataSource = 0;

	if ( m_ui->datasourceList->count() )
	{
		QListWidgetItem *item = m_ui->datasourceList->currentItem();
		if (!item)
			item = m_ui->datasourceList->item(0);
		BaseDataSource *ds = PtrVariant<BaseDataSource>::asPtr(item->data(DATASOURCE_ROLE));
		switch( ds->dataSource )
		{
		case LOCAL: {
			LocalDataSource *s = new LocalDataSource;

			dataSource = s;
			break;
		}
		case FTP: {
			FtpDataSource *s = new FtpDataSource;

			dataSource = s;
			break;
		}
		default:
			break;
		}
	} else {
		LocalDataSource *s = new LocalDataSource;

		dataSource = s;
	}

	if( dataSource != 0 )
	{
		AddEditDataSource *addEdit = new AddEditDataSource(dataSource, AddEditDataSource::ADD);

		if( addEdit->exec() == QDialog::Accepted )
		{
			BaseDataSource *ds = addEdit->dataSource();
			QListWidgetItem *item =  new QListWidgetItem(ds->dataSourceIcon(), ds->label);
			item->setData(DATASOURCE_ROLE, PtrVariant<BaseDataSource>::asQVariant(ds));
			item->setData(UNUSED_ROLE, true);
			m_ui->datasourceList->addItem(item);
			m_ui->datasourceList->setCurrentItem(item);
		}
		else
			delete dataSource;

		delete addEdit;
	}

}

void SettingsDialog::editDataSource()
{
	if ( !m_ui->datasourceList->count() )
		return;

	QListWidgetItem *item = m_ui->datasourceList->currentItem();
	if (!item)
		return;

	BaseDataSource *ds = PtrVariant<BaseDataSource>::asPtr(item->data(DATASOURCE_ROLE));
	AddEditDataSource *addEdit = new AddEditDataSource(ds, AddEditDataSource::EDIT);

	if (addEdit->exec())
	{
		BaseDataSource *edited = addEdit->dataSource();

		if ( edited != ds )
		{
			// old datasource is deleted in ~AddEditDataSource
			item->setData(DATASOURCE_ROLE, PtrVariant<BaseDataSource>::asQVariant(edited));
			item->setIcon(edited->dataSourceIcon());
			item->setText(edited->label);
		}
	}

	delete addEdit;
}

void SettingsDialog::removeDataSource()
{
	if ( !m_ui->datasourceList->count() )
		return;

	QListWidgetItem *it = m_ui->datasourceList->currentItem();
	if (!it)
		return;

	//  no need to call deleteLater() on used/application datasource
	//  because unused datasources are deleted in
	//  mainwindow.cpp
	BaseDataSource *ds = PtrVariant<BaseDataSource>::asPtr(it->data(DATASOURCE_ROLE));
	if (it->data(UNUSED_ROLE).toBool())
	{
		ds->deleteLater();
	}

	int row = m_ui->datasourceList->currentRow();
	m_ui->datasourceList->takeItem(row);

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
	}
}

void SettingsDialog::setupDatasourceList()
{
	m_ui->datasourceList->clear();
    if (Settings::get()->DataSources.isEmpty())
		return;

    foreach(BaseDataSource *s, Settings::get()->DataSources)
	{
		QListWidgetItem *i = new QListWidgetItem(s->dataSourceIcon(), s->label);
		i->setData(DATASOURCE_ROLE, PtrVariant<BaseDataSource>::asQVariant(s));
		i->setData(UNUSED_ROLE, false);
		m_ui->datasourceList->addItem(i);
	}

	m_ui->datasourceList->setCurrentRow(0);
	m_ui->datasourceList->setFocus();
}

void SettingsDialog::pruneCache(QString path)
{
	if(path.isEmpty())
		path = BaseRemoteDataSource::cacheDirPath();

	QDir dir(path);

	QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

	foreach(QFileInfo f, files)
	{
		if(f.isDir())
			pruneCache(f.filePath());
		else
			QFile::remove(f.filePath());
	}

	dir.rmdir(path);
}

void SettingsDialog::setZimaUtilPath(int util)
{
	QString path = QFileDialog::getOpenFileName(this, tr("ZIMA-CAD-Parts - set %1 path").arg(ZimaUtils::labelForUtility(util)), zimaUtilLineEdits[util]->text());

	if (!path.isEmpty())
		zimaUtilLineEdits[util]->setText(path);
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

void SettingsDialog::productViewButton_clicked()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("ZIMA-CAD-Parts - set ProductView path"),
	              m_ui->productViewEdit->text());
	if (!str.isEmpty())
		m_ui->productViewEdit->setText(str);
}
