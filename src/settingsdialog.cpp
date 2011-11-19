#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QApplication>

#include "addeditdatasource.h"

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
//	connect(m_ui->txtHost, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
//	connect(m_ui->txtPort, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
//	connect(m_ui->txtBaseDir, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
//	connect(m_ui->txtLogin, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
//	connect(m_ui->txtPass, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
	//connect(m_ui->pathLineEdit, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
//	connect(m_ui->pathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changingText()));
//	connect(m_ui->checkPassive, SIGNAL(toggled(bool)), this, SLOT(changingPassive()));

	//servers = settings->loadDataSources();
	updateServerList();

	m_ui->spinPicture->setValue( settings->value("GUI/ThumbWidth", 32).toInt() );
	m_ui->previewWidthSpinBox->setValue( settings->value("GUI/PreviewWidth", 256).toInt() );
	m_ui->languageComboBox->setCurrentIndex( langIndex( settings->value("Language", "detect").toString() ) );
	m_ui->splashGroupBox->setChecked( settings->value("GUI/Splash/Enabled", true).toBool() );
	m_ui->splashDurationSpinBox->setValue( settings->value("GUI/Splash/Duration", 1500).toInt() );
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
//	currentSettings = settings;



//	m_ui->spinPicture->setValue(settings->value("GUI/thumbWidth", 64).toInt());

//	updateServerList();
}

void SettingsDialog::saveSettings()
{

	settings->setValue("GUI/ThumbWidth", m_ui->spinPicture->value());
	settings->setValue("GUI/PreviewWidth", m_ui->previewWidthSpinBox->value());
	settings->setValue("GUI/Splash/Enabled", m_ui->splashGroupBox->isChecked());
	settings->setValue("GUI/Splash/Duration", m_ui->splashDurationSpinBox->value());

	QString lang = langIndexToName( m_ui->languageComboBox->currentIndex() );
	if( lang != settings->value("Language").toString() )
	{
		qApp->removeTranslator(*translator);

		if( lang != "detect" )
		{
			QTranslator *t = new QTranslator(parent());
			QString filename = qApp->arguments()[0] + "_" + lang;
			QStringList paths;

			paths
					<< filename
					<< ("locale/" + filename)
					<< (":/" + filename);

			foreach(QString path, paths)
				if( t->load(path) )
				{
					qApp->installTranslator(t);
					*translator = t;
					break;
				}
		}
	}

	settings->setValue("Language", lang);
}

void SettingsDialog::addDataSource()
{
	//	FtpServer *s = new FtpServer();

	//	s->address = "localhost";
	//	servers.append(s);
	//	updateServerList();

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
		}

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

//	servers.remove(m_ui->listFtp->currentRow());
//	delete m_ui->listFtp->currentItem();
//	updateServerList();

	QListWidgetItem *it = m_ui->listFtp->currentItem();
	int row = m_ui->listFtp->currentRow();

	m_ui->listFtp->takeItem(row);
	servers.remove(row);
	delete it;

//	if ( m_ui->listFtp->count() > 0 )
//	{
//		m_ui->listFtp->setCurrentRow( row > 0 ? --row : row);
//	}
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
	}/* else {
		currentServer = new LocalDataSource;
		originServers.append(currentServer);
		updateServerList();
	}*/

	m_ui->listFtp->setFocus();
}

//void SettingsDialog::changingText()
//{
//	if (!currentServer)
//		return;

//	switch( currentServer->dataSource )
//	{
//	case LOCAL: {
//		LocalDataSource *s = static_cast<LocalDataSource*>(currentServer);

//		s->localPath = m_ui->pathLineEdit->text();

//		if (currentServer->lwItem)
//			currentServer->lwItem->setText(m_ui->pathLineEdit->text());
//		break;
//	}
//	case FTP: {
//		FtpDataSource *ftpCurrentServer = static_cast<FtpDataSource*>(currentServer);

//		ftpCurrentServer->remoteHost = m_ui->txtHost->text();
//		ftpCurrentServer->remotePort = m_ui->txtPort->text().toInt();
//		ftpCurrentServer->remoteBaseDir = m_ui->txtBaseDir->text();
//		ftpCurrentServer->remoteLogin = m_ui->txtLogin->text();
//		ftpCurrentServer->remotePassword = m_ui->txtPass->text();

//		if (currentServer->lwItem)
//			currentServer->lwItem->setText(m_ui->txtHost->text());
//		break;
//	}
//	default:
//		break;
//	}
//}

//void SettingsDialog::changingPassive()
//{
//	if (!currentServer)
//		return;
////	FtpDataSource *ftpCurrentServer = static_cast<FtpDataSource*>(currentServer);
////	ftpCurrentServer->ftpPassiveMode = m_ui->checkPassive->isChecked();
//}

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
