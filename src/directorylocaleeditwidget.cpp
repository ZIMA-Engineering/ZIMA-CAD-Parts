#include "directorylocaleeditwidget.h"
#include "ui_directorylocaleeditwidget.h"
#include "metadata.h"

DirectoryLocaleEditWidget::DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DirectoryLocaleEditWidget),
	m_lang(lang)
{
	ui->setupUi(this);

	ui->labelLineEdit->setText(meta->getLabel(lang));
}

DirectoryLocaleEditWidget::~DirectoryLocaleEditWidget()
{
	delete ui;
}

QString DirectoryLocaleEditWidget::label() const
{
	return ui->labelLineEdit->text();
}

void DirectoryLocaleEditWidget::apply(Metadata *meta)
{
	meta->setLabel(m_lang, ui->labelLineEdit->text());
}
