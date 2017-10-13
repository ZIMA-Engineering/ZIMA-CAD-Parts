#include "directorylocaleeditwidget.h"
#include "ui_directorylocaleeditwidget.h"
#include "metadata.h"
#include "directoryeditcolumnmodel.h"

#include <QDebug>

DirectoryLocaleEditWidget::DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, const QStringList &primaryColumns, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DirectoryLocaleEditWidget),
	m_lang(lang)
{
	ui->setupUi(this);

	ui->labelLineEdit->setText(meta->getLabel(lang));
	ui->columnListView->setModel(new DirectoryEditColumnModel(meta->dataColumnLabels(lang), primaryColumns, this));

	connect(ui->addColumnButton, SIGNAL(clicked()),
			this, SLOT(addNewColumn()));
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

void DirectoryLocaleEditWidget::addPrimaryColumn(int row)
{
	QModelIndex root = ui->columnListView->rootIndex();
	auto model = ui->columnListView->model();

	// If true, this widget is where the signal originated, so we
	// do not want to add another column
	if (row > model->rowCount(root))
		return;

	ui->columnListView->model()->insertRows(row, 1, root);
}

void DirectoryLocaleEditWidget::addNewColumn()
{
	QModelIndex root = ui->columnListView->rootIndex();
	auto model = ui->columnListView->model();
	int row = model->rowCount(root);

	if (!ui->columnListView->model()->insertRows(row, 1, root))
		return;

	QModelIndex current = model->index(row, 0, root);
	ui->columnListView->setCurrentIndex(current);
	ui->columnListView->edit(current);

	emit primaryColumnAdded(row);
}
