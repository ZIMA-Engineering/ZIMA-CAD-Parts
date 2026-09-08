#include <QButtonGroup>
#include <QEvent>
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

        QPushButton *flag = new QPushButton(QIcon(QString(":/gfx/flags/%1.%2").arg(langCode, langCode == "fr" ? "svg" : "png")), "", this);
        const QStringList names = {QStringLiteral("English"), QString::fromUtf8("Čeština"),
                                   QStringLiteral("Deutsch"), QString::fromUtf8("Français"), QString::fromUtf8("Русский")};
        flag->setToolTip(names.at(Settings::get()->Languages.indexOf(lang)));
        flag->setAccessibleName(flag->toolTip());
        flag->setFlat(true);
        flag->setCheckable(true);
        flag->setStyleSheet("width: 16px; height: 16px; margin: 0; padding: 1px;");

        if(currentLang == langCode)
            flag->setChecked(true);

        m_buttons->addButton(flag, Settings::get()->Languages.indexOf(lang));

        lay->addWidget(flag);
    }

    setLayout(lay);

    connect(m_buttons, SIGNAL(idClicked(int)),
            this, SLOT(changeLanguage(int)));

    int langIndex = Settings::get()->langIndex(Settings::get()->getCurrentLanguageCode()) - 1;
    // this needs to go after connect to emit the signal
    if (auto button = m_buttons->button(langIndex))
        button->setChecked(true);
}

void LanguageFlagsWidget::changeLanguage(int langIndex)
{
    QString lang = Settings::get()->langIndexToName(langIndex+1);
    Settings::get()->LanguageMetadata = lang.left(2);
    Settings::get()->setCurrentLanguageCode(lang);
    MetadataCache::get()->clear();
}

void LanguageFlagsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        const int index = Settings::get()->langIndex(Settings::get()->getCurrentLanguageCode()) - 1;
        if (auto button = m_buttons->button(index))
            button->setChecked(true);
    }
    QWidget::changeEvent(event);
}
