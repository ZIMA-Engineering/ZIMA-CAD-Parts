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
	m_parameters = m_meta->parameterHandles();

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

	ui->subdirPartsCheckBox->setChecked(m_meta->showDirectoriesAsParts());
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
			MetadataCache::get()->clearBelow(m_dirPath);
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

	// Subdirectory parts
	MetadataCache::get()->metadata(m_dirPath)->setShowDirectoriesAsParts(
		ui->subdirPartsCheckBox->isChecked()
	);

	// Parameter handles
	if (!m_deletedParameters.isEmpty())
	{
		foreach (const QString &handle, m_deletedParameters)
			MetadataCache::get()->metadata(m_dirPath)->removeParameter(handle);
	}

	if (!m_handleChanges.isEmpty())
	{
		QHashIterator<QString, QString> i(m_handleChanges);

		while (i.hasNext())
		{
			i.next();
			MetadataCache::get()->metadata(m_dirPath)->renameParameter(
				i.key(),
				i.value()
			);
		}
	}

	MetadataCache::get()->metadata(m_dirPath)->setParameterHandles(m_parameters);

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

	foreach (const QString &code, languages)
	{
		QLocale locale(code);
		QString langCode = code.left(2);

		ui->languageComboBox->addItem(
			QIcon(QString(":/gfx/flags/%1.png").arg(langCode)),
			locale.nativeLanguageName(),
			code
		);

		auto w = new DirectoryLocaleEditWidget(m_meta, langCode);

		connect(this, SIGNAL(parameterAdded(QString)),
				w, SLOT(addParameter(QString)));
		connect(w, SIGNAL(parameterAdded(QString)),
				this, SLOT(parameterAddition(QString)));
		connect(w, SIGNAL(parameterAdded(QString)),
				this, SIGNAL(parameterAdded(QString)));

		connect(this, SIGNAL(parameterHandleChanged(QString,QString)),
				w, SLOT(parameterHandleChange(QString,QString)));
		connect(w, SIGNAL(parameterHandleChanged(QString,QString)),
				this, SLOT(parameterHandleChange(QString,QString)));
		connect(w, SIGNAL(parameterHandleChanged(QString,QString)),
				this, SIGNAL(parameterHandleChanged(QString,QString)));

		connect(this, SIGNAL(parameterRemoved(QString)),
				w, SLOT(removeParameter(QString)));
		connect(w, SIGNAL(parameterRemoved(QString)),
				this, SLOT(parameterRemoval(QString)));
		connect(w, SIGNAL(parameterRemoved(QString)),
				this, SIGNAL(parameterRemoved(QString)));

		connect(this, SIGNAL(parametersReordered(QStringList)),
				w, SLOT(reorderParameters(QStringList)));
		connect(w, SIGNAL(parametersReordered(QStringList)),
				this, SLOT(reorderParameters(QStringList)));
		connect(w, SIGNAL(parametersReordered(QStringList)),
				this, SIGNAL(parametersReordered(QStringList)));

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

void DirectoryEditorDialog::parameterAddition(const QString &handle)
{
	m_parameters << handle;
}

void DirectoryEditorDialog::parameterHandleChange(const QString &handle, const QString &newHandle)
{
	QString key;
	m_parameters.replace(m_parameters.indexOf(handle), newHandle);

	if (!(key = m_handleChanges.key(handle)).isEmpty())
	{
		// Changing previously changed key
		if (!m_meta->parameterHandles().contains(key))
			return;

		m_handleChanges[key] = newHandle;
		return;
	}

	if (!m_meta->parameterHandles().contains(handle))
		return;

	m_handleChanges[handle] = newHandle;
}

void DirectoryEditorDialog::parameterRemoval(const QString &handle)
{
	QString key;

	if (!(key = m_handleChanges.key(handle)).isEmpty())
	{
		// Removing previously renamed parameter
		// We need to remove the original name
		m_parameters.removeOne(handle);
		m_deletedParameters << key;
		m_handleChanges.remove(key);
		return;
	}

	m_parameters.removeOne(handle);
	m_deletedParameters << handle;
}

void DirectoryEditorDialog::reorderParameters(const QStringList &parameters)
{
	m_parameters = parameters;
}
