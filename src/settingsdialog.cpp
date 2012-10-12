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

#include "addeditdatasource.h"
#include "baseremotedatasource.h"

SettingsDialog::SettingsDialog(QSettings *settings, QVector<BaseDataSource*> servers, QTranslator **translator, QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SettingsDialog),
	settings(settings),
	originServers(servers),
	translator(translator)
{
	m_ui->setupUi(this);

	connect(m_ui->btnAdd, SIGNAL(clicked()), this, SLOT(addDataSource()));
	connect(m_ui->editBtn, SIGNAL(clicked()), this, SLOT(editDataSource()));
	connect(m_ui->btnRemove, SIGNAL(clicked()), this, SLOT(removeDataSource()));
	connect(m_ui->listFtp, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectDataSource(QListWidgetItem*,QListWidgetItem*)));
	connect(m_ui->pruneCacheButton, SIGNAL(clicked()), this, SLOT(pruneCache()));

	updateServerList();

	m_ui->spinPicture->setValue( settings->value("GUI/ThumbWidth", 32).toInt() );
	m_ui->previewWidthSpinBox->setValue( settings->value("GUI/PreviewWidth", 256).toInt() );
	m_ui->languageComboBox->setCurrentIndex( langIndex( settings->value("Language", "detect").toString() ) );
	m_ui->splashGroupBox->setChecked( settings->value("GUI/Splash/Enabled", true).toBool() );
	m_ui->splashDurationSpinBox->setValue( settings->value("GUI/Splash/Duration", 1500).toInt() );
	m_ui->developerModeGroupBox->setChecked( settings->value("Developer/Enabled", false).toBool() );
	m_ui->techSpecToolBarCheckBox->setChecked( settings->value("Developer/TechSpecToolBar", true).toBool() );
	m_ui->dirTreePathCheckBox->setChecked( settings->value("Developer/DirTreePath", true).toBool() );

#ifdef INCLUDE_PRODUCT_VIEW
	productViewSettings = new ProductViewSettings(settings, this);
	m_ui->tabWidget->addTab(productViewSettings, tr("ProductView Settings"));
#endif // INCLUDE_PRODUCT_VIEW
}

SettingsDialog::~SettingsDialog()
{
	servers.clear();

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

void SettingsDialog::loadSettings(QSettings *settings)
{
}

void SettingsDialog::saveSettings()
{
	settings->setValue("GUI/ThumbWidth", m_ui->spinPicture->value());
	settings->setValue("GUI/PreviewWidth", m_ui->previewWidthSpinBox->value());
	settings->setValue("GUI/Splash/Enabled", m_ui->splashGroupBox->isChecked());
	settings->setValue("GUI/Splash/Duration", m_ui->splashDurationSpinBox->value());
	settings->setValue("Developer/Enabled", m_ui->developerModeGroupBox->isChecked());
	settings->setValue("Developer/TechSpecToolBar", m_ui->techSpecToolBarCheckBox->isChecked());
	settings->setValue("Developer/DirTreePath", m_ui->dirTreePathCheckBox->isChecked());

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

#ifdef INCLUDE_PRODUCT_VIEW
	productViewSettings->saveSettings();
#endif // INCLUDE_PRODUCT_VIEW
}

void SettingsDialog::addDataSource()
{
	BaseDataSource *dataSource = 0;

	if ( m_ui->listFtp->count() )
	{
		switch( currentServer->dataSource )
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
			currentServer = addEdit->dataSource();

			currentServer->lwItem = new QListWidgetItem( currentServer->dataSourceIcon(), currentServer->label );

			servers << currentServer;

			m_ui->listFtp->addItem(currentServer->lwItem);
			m_ui->listFtp->setCurrentItem(currentServer->lwItem);
		} else delete dataSource;

		delete addEdit;
	}

}

void SettingsDialog::editDataSource()
{
	if ( !m_ui->listFtp->count() )
		return;

	AddEditDataSource *addEdit = new AddEditDataSource(currentServer, AddEditDataSource::EDIT);

	if( addEdit->exec() == QDialog::Accepted )
	{
		BaseDataSource *edited = addEdit->dataSource();

		if( edited != currentServer )
		{
			servers.remove( m_ui->listFtp->currentRow() );
			servers.insert( m_ui->listFtp->currentRow(), edited );

			// old currentServer is deleted in ~AddEditDataSource
			currentServer = edited;
			currentServer->lwItem->setIcon( currentServer->dataSourceIcon() );
		}

		currentServer->lwItem->setText( currentServer->label );
	}

	delete addEdit;
}

void SettingsDialog::removeDataSource()
{
	if ( !m_ui->listFtp->count() )
		return;

	QListWidgetItem *it = m_ui->listFtp->currentItem();
	int row = m_ui->listFtp->currentRow();

	m_ui->listFtp->takeItem(row);
	servers.remove(row);
	delete it;
}

void SettingsDialog::selectDataSource(QListWidgetItem *current, QListWidgetItem *)
{
	if (!current)
		return;

	currentServer = servers[ m_ui->listFtp->currentRow() ];
}

void SettingsDialog::updateServerList()
{
	m_ui->listFtp->clear();

	foreach(BaseDataSource *s, originServers)
	{
		QListWidgetItem *lwi = new QListWidgetItem(s->dataSourceIcon(), s->label);
		s->lwItem = lwi;

		servers << s;

		m_ui->listFtp->addItem(lwi);
	}

	if(!servers.isEmpty())
	{
		currentServer = servers.first();
		m_ui->listFtp->setCurrentItem(currentServer->lwItem);
	}

	m_ui->listFtp->setFocus();
}

QVector<BaseDataSource*> SettingsDialog::getData()
{
	return servers;
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
