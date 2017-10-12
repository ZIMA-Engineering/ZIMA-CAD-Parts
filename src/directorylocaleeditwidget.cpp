#include "directorylocaleeditwidget.h"
#include "ui_directorylocaleeditwidget.h"
#include "metadata.h"
#include "directoryeditcolumnmodel.h"

DirectoryLocaleEditWidget::DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, const QStringList &primaryColumns, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DirectoryLocaleEditWidget),
	m_lang(lang)
{
	ui->setupUi(this);

	ui->labelLineEdit->setText(meta->getLabel(lang));
	ui->columnListView->setModel(new DirectoryEditColumnModel(meta->dataColumnLabels(lang), primaryColumns, this));
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

	auto model = static_cast<DirectoryEditColumnModel*>(ui->columnListView->model());

	meta->setDataColumnLabels(m_lang, model->columnLabels());
}
