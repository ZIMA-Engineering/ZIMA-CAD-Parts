#include "directoryeditordialog.h"
#include "ui_directoryeditordialog.h"
#include "settings.h"
#include "metadata.h"
#include "directorylocaleeditwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

DirectoryEditorDialog::DirectoryEditorDialog(const QFileInfo &fi, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DirectoryEditorDialog),
	m_fi(fi)
{
	ui->setupUi(this);

	m_meta = MetadataCache::get()->metadata(fi.absoluteFilePath());
	m_dirPath = m_fi.absoluteFilePath();

	setupLanguageBox();
	connect(ui->languageComboBox, SIGNAL(currentIndexChanged(int)),
			ui->stackedWidget, SLOT(setCurrentIndex(int)));

	ui->nameLineEdit->setText(fi.fileName());

	// No idea why is this needed, but the stack widget's background
	// is white without this.
	ui->stackedWidget->setStyleSheet("background-color: pallete(window);");

	setupIcon();
	connect(ui->removeIconButton, SIGNAL(clicked()),
			this, SLOT(removeIcon()));
	connect(ui->iconButton, SIGNAL(clicked()),
			this, SLOT(openIconDialog()));
}

DirectoryEditorDialog::~DirectoryEditorDialog()
{
	delete ui;
}

void DirectoryEditorDialog::apply()
{
	// Directory name
	QString name = ui->nameLineEdit->text().trimmed();

	if (name != m_fi.fileName())
	{
		QString dstDirPath = m_fi.absolutePath() +"/"+ name;

		if (QFile::rename(m_dirPath, dstDirPath))
		{
			MetadataCache::get()->clear(m_dirPath);
			m_dirPath = dstDirPath;

		} else {
			QMessageBox::warning(
				this,
				tr("Unable to rename directory"),
				tr("Unable to rename directory '%1'").arg(m_dirPath)
			);
		}
	}

	// Directory icon
	if (m_iconPath != m_origIconPath)
	{
		if (m_iconPath.isEmpty())
			uninstallIcon();
		else
			installIcon(m_iconPath, hasAnyLabel() ? LOGO_TEXT_FILE : LOGO_FILE);

	} else if (hasIcon(LOGO_FILE) && hasAnyLabel()) {
		installIcon(iconInstallPath(LOGO_FILE), LOGO_TEXT_FILE, true);

	} else if (hasIcon(LOGO_TEXT_FILE) && !hasAnyLabel()) {
		installIcon(iconInstallPath(LOGO_TEXT_FILE), LOGO_FILE, true);
	}

	// Locales
	int cnt = ui->stackedWidget->count();

	for (int i = 0; i < cnt; i++)
	{
		static_cast<DirectoryLocaleEditWidget*>(ui->stackedWidget->widget(i))->apply(
			MetadataCache::get()->metadata(m_dirPath)
		);
	}
}

void DirectoryEditorDialog::setupLanguageBox()
{
	QStringList languages = Settings::get()->Languages;
	QStringList primaryLangColumns = findPrimaryLanguageColumns(languages);

	foreach (const QString &code, languages)
	{
		QLocale locale(code);
		QString langCode = code.left(2);

		ui->languageComboBox->addItem(
			QIcon(QString(":/gfx/flags/%1.png").arg(langCode)),
			locale.nativeLanguageName(),
			code
		);

		auto w = new DirectoryLocaleEditWidget(
			m_meta,
			langCode,
			primaryLangColumns
		);

		connect(this, SIGNAL(primaryColumnAdded(int)),
				w, SLOT(addPrimaryColumn(int)));
		connect(w, SIGNAL(primaryColumnAdded(int)),
				this, SIGNAL(primaryColumnAdded(int)));

		ui->stackedWidget->addWidget(w);
	}

	int i = languages.indexOf(Settings::get()->getCurrentLanguageCode());

	ui->languageComboBox->setCurrentIndex(i);
	ui->stackedWidget->setCurrentIndex(i);
}

void DirectoryEditorDialog::setupIcon()
{
	QString name;

	if (hasIcon(LOGO_FILE))
	{
		name = LOGO_FILE;

	} else if (hasIcon(LOGO_TEXT_FILE)) {
		name = LOGO_TEXT_FILE;
	}

	if (name.isNull())
	{
		ui->iconLabel->setText(tr("No icon"));
		ui->removeIconButton->hide();

	} else {
		QString path = iconInstallPath(name);
		setIcon(path);
		m_iconPath = path;
		m_origIconPath = path;
	}
}

void DirectoryEditorDialog::setIcon(const QString &path)
{
	ui->iconLabel->setPixmap(QPixmap(path));
}

void DirectoryEditorDialog::installIcon(const QString &icon, const QString &name, bool rename)
{
	QString dstPath = iconInstallPath(name);
	bool ok;

	if (rename)
		ok = QFile::rename(icon, dstPath);
	else
		ok = QFile::copy(icon, dstPath);

	if (!ok)
	{
		QMessageBox::warning(
			this,
			tr("Unable to copy icon"),
			tr("Unable to copy '%1' to '%2'").arg(m_dirPath).arg(dstPath)
		);
	}
}

void DirectoryEditorDialog::uninstallIcon()
{
	if (!QFile::remove(m_origIconPath))
	{
		QMessageBox::warning(
			this,
			tr("Unable to remove icon"),
			tr("Unable to remove icon '%1'").arg(m_origIconPath)
		);
	}
}

bool DirectoryEditorDialog::hasAnyLabel()
{
	int cnt = ui->stackedWidget->count();

	for (int i = 0; i < cnt; i++)
	{
		auto w = static_cast<DirectoryLocaleEditWidget*>(ui->stackedWidget->widget(i));

		if (!w->label().isEmpty())
			return true;
	}

	return false;
}

bool DirectoryEditorDialog::hasIcon(const QString &name) const
{
	return QFile::exists(iconInstallPath(name));
}

QString DirectoryEditorDialog::iconInstallPath(const QString &name) const
{
	return m_dirPath +"/"+ TECHSPEC_DIR + "/" + name;
}

QStringList DirectoryEditorDialog::findPrimaryLanguageColumns(QStringList languages)
{
	int maxCols = -1;
	QStringList ret;

	foreach (const QString &code, languages)
	{
		QStringList cols = m_meta->dataColumnLabels(code.left(2));

		if (cols.count() > maxCols)
		{
			maxCols = cols.count();
			ret = cols;
		}
	}

	return ret;
}

void DirectoryEditorDialog::removeIcon()
{
	m_iconPath.clear();
	ui->iconLabel->setPixmap(QPixmap());
	ui->removeIconButton->hide();
}

void DirectoryEditorDialog::openIconDialog()
{
	QString iconFile = QFileDialog::getOpenFileName(
		this,
		tr("Select icon"),
		"",
		"Images (*.png *.jpg *.jpeg *.gif)"
	);

	if (iconFile.isNull())
		return;

	setIcon(iconFile);
	m_iconPath = iconFile;
	ui->removeIconButton->show();
}
