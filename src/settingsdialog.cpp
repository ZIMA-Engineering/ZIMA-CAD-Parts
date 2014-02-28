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


SettingsDialog::SettingsDialog(QSettings *settings, QList<BaseDataSource*> datasources, QTranslator **translator, QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SettingsDialog),
	settings(settings),
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
	connect(m_ui->datasourceList, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(editDataSource()));

	setupDatasourceList(datasources);

	m_ui->spinPicture->setValue( settings->value("GUI/ThumbWidth", 32).toInt() );
	m_ui->previewWidthSpinBox->setValue( settings->value("GUI/PreviewWidth", 256).toInt() );
	m_ui->languageComboBox->setCurrentIndex( langIndex( settings->value("Language", "detect").toString() ) );
	m_ui->splashGroupBox->setChecked( settings->value("GUI/Splash/Enabled", true).toBool() );
	m_ui->splashDurationSpinBox->setValue( settings->value("GUI/Splash/Duration", 1500).toInt() );
	m_ui->developerModeGroupBox->setChecked( settings->value("Developer/Enabled", false).toBool() );
	m_ui->techSpecToolBarCheckBox->setChecked( settings->value("Developer/TechSpecToolBar", true).toBool() );
	m_ui->productViewEdit->setText(settings->value("Extensions/ProductView/Path", PRODUCT_VIEW_DEFAULT_PATH).toString());

	connect(m_ui->proeButton, SIGNAL(clicked()),
	        this, SLOT(proeButton_clicked()));

	zimaUtilSignalMapper = new QSignalMapper(this);

	connect(zimaUtilSignalMapper, SIGNAL(mapped(int)), this, SLOT(setZimaUtilPath(int)));

	settings->beginGroup("ExternalPrograms");

	for(int i = 0; i < ZimaUtils::ZimaUtilsCount; i++)
	{
		settings->beginGroup(ZimaUtils::internalNameForUtility(i));

		QToolButton *t = new QToolButton(this);
		t->setText("...");

		connect(t, SIGNAL(clicked()), zimaUtilSignalMapper, SLOT(map()));

		zimaUtilLineEdits << new QLineEdit(settings->value("Executable").toString(), this);

		m_ui->gridLayout->addWidget(new QLabel(ZimaUtils::labelForUtility(i), this), i, 0);
		m_ui->gridLayout->addWidget(zimaUtilLineEdits.last(), i, 1);
		m_ui->gridLayout->addWidget(t, i, 2);

		zimaUtilSignalMapper->setMapping(t, i);

		settings->endGroup();
	}

	settings->beginGroup("ProE");
	m_ui->proeEdit->setText(settings->value("Executable", "proe.exe").toString());
	settings->endGroup();

	settings->endGroup();
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

void SettingsDialog::loadSettings(QSettings *settings)
{
	Q_UNUSED(settings);
}

void SettingsDialog::saveSettings()
{
	settings->setValue("GUI/ThumbWidth", m_ui->spinPicture->value());
	settings->setValue("GUI/PreviewWidth", m_ui->previewWidthSpinBox->value());
	settings->setValue("GUI/Splash/Enabled", m_ui->splashGroupBox->isChecked());
	settings->setValue("GUI/Splash/Duration", m_ui->splashDurationSpinBox->value());
	settings->setValue("Developer/Enabled", m_ui->developerModeGroupBox->isChecked());
	settings->setValue("Developer/TechSpecToolBar", m_ui->techSpecToolBarCheckBox->isChecked());
	settings->setValue("Extensions/ProductView/Path", m_ui->productViewEdit->text());


	QString lang = langIndexToName( m_ui->languageComboBox->currentIndex() );
	if( lang != settings->value("Language").toString() )
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

	settings->setValue("Language", lang);

	settings->beginGroup("ExternalPrograms");

	for(int i = 0; i < ZimaUtils::ZimaUtilsCount; i++)
	{
		settings->beginGroup(ZimaUtils::internalNameForUtility(i));
		settings->setValue("Executable", zimaUtilLineEdits[i]->text());
		settings->endGroup();
	}

	settings->beginGroup("ProE");
	settings->setValue("Executable", m_ui->proeEdit->text());
	settings->endGroup();

	settings->endGroup();
}

void SettingsDialog::addDataSource()
{
	BaseDataSource *dataSource = 0;

	if ( m_ui->datasourceList->topLevelItemCount() )
	{
		QTreeWidgetItem *item = m_ui->datasourceList->currentItem();
		if (!item)
			item = m_ui->datasourceList->topLevelItem(0);
		BaseDataSource *ds = PtrVariant<BaseDataSource>::asPtr(item->data(0, DATASOURCE_ROLE));
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
		AddEditDataSource *addEdit = new AddEditDataSource(dataSource, AddEditDataSource::ADD, allGroups());

		if( addEdit->exec() == QDialog::Accepted )
		{
			BaseDataSource *ds = addEdit->dataSource();
			QTreeWidgetItem *item =  new QTreeWidgetItem(QStringList() << ds->label << ds->group());
			item->setIcon(0, ds->dataSourceIcon());
			item->setData(0, DATASOURCE_ROLE, PtrVariant<BaseDataSource>::asQVariant(ds));
			item->setData(0, UNUSED_ROLE, true);
			m_ui->datasourceList->addTopLevelItem(item);
			m_ui->datasourceList->setCurrentItem(item);
		}
		else
			delete dataSource;

		delete addEdit;
	}

}

void SettingsDialog::editDataSource()
{
	if ( !m_ui->datasourceList->topLevelItemCount() )
		return;

	QTreeWidgetItem *item = m_ui->datasourceList->currentItem();
	if (!item)
		return;

	BaseDataSource *ds = PtrVariant<BaseDataSource>::asPtr(item->data(0, DATASOURCE_ROLE));
	AddEditDataSource addEdit(ds, AddEditDataSource::EDIT, allGroups());

	if (!addEdit.exec())
		return;

	BaseDataSource *edited = addEdit.dataSource();
	// old datasource is deleted in ~AddEditDataSource
	item->setData(0, DATASOURCE_ROLE, PtrVariant<BaseDataSource>::asQVariant(edited));
	item->setIcon(0, edited->dataSourceIcon());
	item->setText(0, edited->label);
	item->setText(1, edited->group());
}

void SettingsDialog::removeDataSource()
{
	if ( !m_ui->datasourceList->topLevelItemCount() )
		return;

	QTreeWidgetItem *it = m_ui->datasourceList->currentItem();
	if (!it)
		return;

	//  no need to call deleteLater() on used/application datasource
	//  because unused datasources are deleted in
	//  mainwindow.cpp
	BaseDataSource *ds = PtrVariant<BaseDataSource>::asPtr(it->data(0, DATASOURCE_ROLE));
	if (it->data(0, UNUSED_ROLE).toBool())
	{
		ds->deleteLater();
	}

	int row = m_ui->datasourceList->indexOfTopLevelItem(it);
	m_ui->datasourceList->takeTopLevelItem(row);

	delete it;
}

void SettingsDialog::datasourceUpButton_clicked()
{
	QTreeWidget *lw = m_ui->datasourceList;
	QTreeWidgetItem *current = lw->currentItem();
	int ix = lw->indexOfTopLevelItem(current);
	if (ix > 0)
	{
		QTreeWidgetItem *temp = lw->takeTopLevelItem(ix);
		lw->insertTopLevelItem(ix-1, temp);
		lw->setCurrentItem(temp);
	}
}

void SettingsDialog::datasourceDownButton_clicked()
{
	QTreeWidget *lw = m_ui->datasourceList;
	QTreeWidgetItem *current = lw->currentItem();
	int ix = lw->indexOfTopLevelItem(current);
	if (ix < lw->topLevelItemCount()-1)
	{
		QTreeWidgetItem *temp = lw->takeTopLevelItem(ix);
		lw->insertTopLevelItem(ix+1, temp);
		lw->setCurrentItem(temp);
	}
}

void SettingsDialog::setupDatasourceList(QList<BaseDataSource*> datasources)
{
	m_ui->datasourceList->clear();
	if (datasources.isEmpty())
		return;

	QTreeWidgetItem *first = 0;
	foreach(BaseDataSource *s, datasources)
	{
		QTreeWidgetItem *i = new QTreeWidgetItem(QStringList() << s->label << s->group());
		i->setIcon(0, s->dataSourceIcon());
		i->setData(0, DATASOURCE_ROLE, PtrVariant<BaseDataSource>::asQVariant(s));
		i->setData(0, UNUSED_ROLE, false);
		m_ui->datasourceList->addTopLevelItem(i);

		if (!first)
			first = i;
	}

	m_ui->datasourceList->setCurrentItem(first);
	m_ui->datasourceList->setFocus();
	m_ui->datasourceList->resizeColumnToContents(0);
	m_ui->datasourceList->resizeColumnToContents(1);
}

QList<BaseDataSource*> SettingsDialog::getDatasources()
{
	QList<BaseDataSource*> ret;
	for (int i = 0; i < m_ui->datasourceList->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem *item = m_ui->datasourceList->topLevelItem(i);
		ret << PtrVariant<BaseDataSource>::asPtr(item->data(0, DATASOURCE_ROLE));
	}

	return ret;
}

int SettingsDialog::langIndex(QString lang)
{
	if( lang.startsWith("en_") )
		return ENGLISH;
	else if( lang == "cs_CZ" )
		return CZECH;
	else
		return DETECT;
}

QString SettingsDialog::langIndexToName(int lang)
{
	switch(lang)
	{
	case ENGLISH:
		return "en_US";
	case CZECH:
		return "cs_CZ";
	default:
		return "detect";
	}
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

QStringList SettingsDialog::allGroups()
{
	QStringList ret;
	for (int i = 0; i < m_ui->datasourceList->topLevelItemCount(); ++i)
	{
		QString val = m_ui->datasourceList->topLevelItem(i)->text(1);
		if (!ret.contains(val))
			ret << val;
	}

	ret.sort();
	return ret;
}
