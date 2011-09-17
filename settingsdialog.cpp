#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SettingsDialog)
{
	m_ui->setupUi(this);

	connect(m_ui->btnAdd, SIGNAL(clicked()), this, SLOT(addFtpServer()));
	connect(m_ui->btnRemove, SIGNAL(clicked()), this, SLOT(removeFtpServer()));
	connect(m_ui->listFtp, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectFtpServer(QListWidgetItem*,QListWidgetItem*)));
	connect(m_ui->txtHost, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
	connect(m_ui->txtPort, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
	connect(m_ui->txtBaseDir, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
	connect(m_ui->txtLogin, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
	connect(m_ui->txtPass, SIGNAL(textEdited(QString)), this, SLOT(changingText()));
	connect(m_ui->checkPassive, SIGNAL(toggled(bool)), this, SLOT(changingPassive()));
}

SettingsDialog::~SettingsDialog()
{
	qDeleteAll(servers);
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
	QString str;
	settings->beginGroup("ftp");
	foreach(str, settings->childGroups())
	{
		FtpServer *s = new FtpServer();
		settings->beginGroup(str);
		s->address = settings->value("host", "localhost").toString();
		s->port = settings->value("port", "21").toInt();
		s->baseDir = settings->value("baseDir", "/").toString();
		s->login = settings->value("login", "").toString();
		s->password = settings->value("password", "").toString();
		s->passiveMode = settings->value("passive", true).toBool();
		settings->endGroup();
		servers.append(s);
	}
	settings->endGroup();

	m_ui->spinPicture->setValue(settings->value("gui/thumbWidth", 64).toInt());
	m_ui->checkEmpty->setChecked(settings->value("gui/showEmpty", true).toBool());

	updateServerList();
}

void SettingsDialog::saveSettings(QSettings *settings)
{
	int i = 0;
	QString str;
	settings->beginGroup("ftp");
	foreach(FtpServer *s, servers)
	{
		settings->beginGroup(QString::number(i++));
		settings->setValue("host", s->address);
		settings->setValue("port", s->port);
		settings->setValue("baseDir", s->baseDir);
		settings->setValue("login", s->login);
		settings->setValue("password", s->password);
		settings->setValue("passive", s->passiveMode);
		settings->endGroup();
	}
	settings->endGroup();
	settings->setValue("gui/thumbWidth", m_ui->spinPicture->value());
	settings->setValue("gui/showEmpty", m_ui->checkEmpty->isChecked());
}

void SettingsDialog::addFtpServer()
{
	FtpServer *s = new FtpServer();

	s->address = "localhost";
	servers.append(s);
	updateServerList();
}

void SettingsDialog::removeFtpServer()
{
	if (!m_ui->listFtp->count())
		return;

	servers.remove(m_ui->listFtp->currentRow());
	delete m_ui->listFtp->currentItem();
	updateServerList();
}

void SettingsDialog::selectFtpServer(QListWidgetItem *current, QListWidgetItem *)
{
	if (!current)
		return;
	currentServer = servers[m_ui->listFtp->currentRow()];
	m_ui->txtHost->setText(currentServer->address);
	m_ui->txtPort->setText(QString::number(currentServer->port));
	m_ui->txtBaseDir->setText(currentServer->baseDir);
	m_ui->txtLogin->setText(currentServer->login);
	m_ui->txtPass->setText(currentServer->password);
	m_ui->checkPassive->setChecked(currentServer->passiveMode);
}

void SettingsDialog::updateServerList()
{
	m_ui->listFtp->clear();
	foreach(FtpServer *s, servers)
	{
		QListWidgetItem *lwi = new QListWidgetItem(s->address);
		s->lwItem = lwi;
		m_ui->listFtp->addItem(lwi);
	}
	if (!servers.isEmpty())
		currentServer = servers[0];
	else
	{
		currentServer = new FtpServer();
		currentServer->address = "localhost";
		servers.append(currentServer);
		updateServerList();
	}
	m_ui->listFtp->setFocus();
}

void SettingsDialog::changingText()
{
	if (!currentServer)
		return;
	if (currentServer->lwItem)
		currentServer->lwItem->setText(m_ui->txtHost->text());
	currentServer->address = m_ui->txtHost->text();
	currentServer->port = m_ui->txtPort->text().toInt();
	currentServer->baseDir = m_ui->txtBaseDir->text();
	currentServer->login = m_ui->txtLogin->text();
	currentServer->password = m_ui->txtPass->text();
}

void SettingsDialog::changingPassive()
{
	if (!currentServer)
		return;
	currentServer->passiveMode = m_ui->checkPassive->isChecked();
}
