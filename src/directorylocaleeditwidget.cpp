#include "directorylocaleeditwidget.h"
#include "ui_directorylocaleeditwidget.h"
#include "metadata.h"
#include "directoryeditparametersmodel.h"

#include <QDebug>

DirectoryLocaleEditWidget::DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DirectoryLocaleEditWidget),
	m_lang(lang)
{
	ui->setupUi(this);

	auto model = new DirectoryEditParametersModel(
		meta->parameterHandles(),
		meta->parametersWithLabels(lang),
		this
	);

	ui->labelLineEdit->setText(meta->getLabel(lang));
	ui->parameterTreeView->setModel(model);

	connect(ui->addParameterButton, SIGNAL(clicked()),
			this, SLOT(addNewParameter()));
	connect(ui->removeParameterButton, SIGNAL(clicked()),
			this, SLOT(removeSelectedParameter()));
	connect(model, SIGNAL(parameterHandleChanged(QString,QString)),
			this, SIGNAL(parameterHandleChanged(QString,QString)));
	connect(model, SIGNAL(parametersReordered(QStringList)),
			this, SIGNAL(parametersReordered(QStringList)));
	connect(ui->parameterTreeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			this, SLOT(parameterSelected(QModelIndex,QModelIndex)));
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

	auto model = static_cast<DirectoryEditParametersModel*>(ui->parameterTreeView->model());
	QHashIterator<QString, QString> i(model->parameterLabels());

	while (i.hasNext())
	{
		i.next();

		if (i.value().isEmpty())
			continue;

		meta->setParameterLabel(i.key(), m_lang, i.value());
	}
}

void DirectoryLocaleEditWidget::addParameter(const QString &handle)
{
	auto model = static_cast<DirectoryEditParametersModel*>(ui->parameterTreeView->model());

	// If true, this widget is where the signal originated, so we
	// do not want to add another column
	if (model->hasParameter(handle))
		return;

	model->addParameter(handle);
}

void DirectoryLocaleEditWidget::parameterHandleChange(const QString &handle, const QString &newHandle)
{
	auto model = static_cast<DirectoryEditParametersModel*>(ui->parameterTreeView->model());

	// If true, this widget is where the signal originated, so we
	// do not want to add another column
	if (model->hasParameter(newHandle))
		return;

	model->changeHandle(handle, newHandle);
}

void DirectoryLocaleEditWidget::removeSelectedParameter()
{
	 QModelIndex index = ui->parameterTreeView->currentIndex();
	 QString handle = ui->parameterTreeView->model()->data(
		index,
		Qt::DisplayRole
	).toString();

	if (!index.isValid())
		return;

	if (!ui->parameterTreeView->model()->removeRows(index.row(), 1, index.parent()))
		return;

	emit parameterRemoved(handle);
}

void DirectoryLocaleEditWidget::parameterSelected(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous)

	if (!current.isValid())
	{
		ui->removeParameterButton->setEnabled(false);
		return;
	}

	ui->removeParameterButton->setEnabled(true);
}

void DirectoryLocaleEditWidget::removeParameter(const QString &handle)
{
	auto model = static_cast<DirectoryEditParametersModel*>(ui->parameterTreeView->model());

	// If true, this widget is where the signal originated, so we
	// do not want to add another column
	if (!model->hasParameter(handle))
		return;

	model->removeParameter(handle);
}

void DirectoryLocaleEditWidget::reorderParameters(const QStringList &parameters)
{
	auto model = static_cast<DirectoryEditParametersModel*>(ui->parameterTreeView->model());
	model->reorderParameters(parameters);
}

void DirectoryLocaleEditWidget::addNewParameter()
{
	QModelIndex root = ui->parameterTreeView->rootIndex();
	auto model = ui->parameterTreeView->model();
	int row = model->rowCount(root);

	if (!model->insertRows(row, 1, root))
		return;

	QModelIndex current = model->index(row, 0, root);
	ui->parameterTreeView->setCurrentIndex(current);
	ui->parameterTreeView->edit(current);

	emit parameterAdded(model->data(
		model->index(row, 1, root),
		Qt::DisplayRole
	).toString());
}
