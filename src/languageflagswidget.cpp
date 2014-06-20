#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>

#include "languageflagswidget.h"
#include "settings.h"


LanguageFlagsWidget::LanguageFlagsWidget(QWidget *parent) :
	QWidget(parent)
{
	QString currentLang = Settings::get()->getCurrentLanguageCode().left(2);

	m_buttons = new QButtonGroup(this);
	m_buttons->setExclusive(true);

	QHBoxLayout *lay = new QHBoxLayout();
	lay->setSpacing(0);

	foreach (QString lang, Settings::get()->Languages)
	{
		QString langCode = lang.left(2);

		QPushButton *flag = new QPushButton(QIcon(QString(":/gfx/flags/%1.png").arg(langCode)), "", this);
		flag->setFlat(true);
		flag->setCheckable(true);
		flag->setStyleSheet("width: 16px; height: 16px; margin: 0; padding: 1px;");

		if(currentLang == lang)
			flag->setChecked(true);

		m_buttons->addButton(flag, Settings::get()->Languages.indexOf(lang));

		lay->addWidget(flag);
	}

	setLayout(lay);

	connect(m_buttons, SIGNAL(buttonClicked(int)),
	        this, SIGNAL(changeLanguage(int)));

	int langIndex = Settings::get()->langIndex(Settings::get()->getCurrentLanguageCode()) - 1;
	// this needs to go after connect to emit the signal
	m_buttons->button(langIndex)->setChecked(true);
}
