#include "directorylocaleeditwidget.h"
#include "ui_directorylocaleeditwidget.h"
#include "metadata.h"
#include "directoryeditparametersmodel.h"
#include "lineeditvaluedelegate.h"

#include <QDebug>

DirectoryLocaleEditWidget::DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectoryLocaleEditWidget),
    m_lang(lang)
{
    ui->setupUi(this);

    m_model = new DirectoryEditParametersModel(
        meta->parameterHandles(),
        meta->parametersWithLabels(lang),
        this
    );

    ui->labelLineEdit->setText(meta->getLabel(lang));
    ui->parameterTreeView->setModel(m_model);
    ui->parameterTreeView->setItemDelegate(new LineEditValueDelegate(this));

    ui->moveParameterUpButton->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    ui->moveParameterDownButton->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    toggleParameterMoveButtons();

    connect(ui->addParameterButton, SIGNAL(clicked()),
            this, SLOT(addNewParameter()));
    connect(ui->removeParameterButton, SIGNAL(clicked()),
            this, SLOT(removeSelectedParameter()));
    connect(ui->moveParameterUpButton, SIGNAL(clicked()),
            this, SLOT(moveSelectedParameterUp()));
    connect(ui->moveParameterDownButton, SIGNAL(clicked()),
            this, SLOT(moveSelectedParameterDown()));
    connect(m_model, SIGNAL(parameterHandleChanged(QString,QString)),
            this, SIGNAL(parameterHandleChanged(QString,QString)));
    connect(m_model, SIGNAL(parametersReordered(QStringList)),
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

    QHashIterator<QString, QString> i(m_model->parameterLabels());

    while (i.hasNext())
    {
        i.next();

        if (i.value().trimmed().isEmpty())
            continue;

        meta->setParameterLabel(i.key(), m_lang, i.value());
    }
}

void DirectoryLocaleEditWidget::addParameter(const QString &handle)
{
    // If true, this widget is where the signal originated, so we
    // do not want to add another column
    if (m_model->hasParameter(handle))
        return;

    m_model->addParameter(handle);
    toggleParameterMoveButtons();
}

void DirectoryLocaleEditWidget::parameterHandleChange(const QString &handle, const QString &newHandle)
{
    // If true, this widget is where the signal originated, so we
    // do not want to add another column
    if (m_model->hasParameter(newHandle))
        return;

    m_model->changeHandle(handle, newHandle);
}

void DirectoryLocaleEditWidget::removeSelectedParameter()
{
    QModelIndex index = ui->parameterTreeView->currentIndex();
    QString handle = handleForIndex(index);

    if (!index.isValid())
        return;

    if (!m_model->removeRows(index.row(), 1, index.parent()))
        return;

    emit parameterRemoved(handle);
    toggleParameterMoveButtons();
}

void DirectoryLocaleEditWidget::parameterSelected(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    toggleParameterMoveButtons();

    if (!current.isValid())
    {
        ui->removeParameterButton->setEnabled(false);
        return;
    }

    ui->removeParameterButton->setEnabled(true);
}

void DirectoryLocaleEditWidget::removeParameter(const QString &handle)
{
    // If true, this widget is where the signal originated, so we
    // do not want to add another column
    if (!m_model->hasParameter(handle))
        return;

    m_model->removeParameter(handle);
    toggleParameterMoveButtons();
}

void DirectoryLocaleEditWidget::reorderParameters(const QStringList &parameters)
{
    m_model->reorderParameters(parameters);
    toggleParameterMoveButtons();
}

void DirectoryLocaleEditWidget::moveSelectedParameterUp()
{
    QModelIndex index = ui->parameterTreeView->currentIndex();
    QString handle = handleForIndex(index);

    if (!index.isValid())
        return;

    m_model->moveParameter(handle, -1);
    ui->parameterTreeView->setCurrentIndex(m_model->index(index.row() - 1, 1));
}

void DirectoryLocaleEditWidget::moveSelectedParameterDown()
{
    QModelIndex index = ui->parameterTreeView->currentIndex();
    QString handle = handleForIndex(index);

    if (!index.isValid())
        return;

    m_model->moveParameter(handle, +1);
    ui->parameterTreeView->setCurrentIndex(m_model->index(index.row() + 1, 1));
}

QString DirectoryLocaleEditWidget::handleForIndex(const QModelIndex &index)
{
    return m_model->data(
               m_model->index(index.row(), 1),
               Qt::DisplayRole
           ).toString();
}

void DirectoryLocaleEditWidget::toggleParameterMoveButtons()
{
    QModelIndex index = ui->parameterTreeView->currentIndex();
    int row = index.row();
    int count = m_model->rowCount(QModelIndex());
    bool up = false, down = false;

    if (!index.isValid()) {
        up = down = false;
    } else if (row == 0) {
        up = false;
        down = (row + 1) < count;
    } else if ((row + 1) == count) {
        up = true;
        down = false;
    } else {
        up = down = true;
    }

    ui->moveParameterUpButton->setEnabled(up);
    ui->moveParameterDownButton->setEnabled(down);
}

void DirectoryLocaleEditWidget::addNewParameter()
{
    QModelIndex root = ui->parameterTreeView->rootIndex();
    int row = m_model->rowCount(root);

    if (!m_model->insertRows(row, 1, root))
        return;

    QModelIndex current = m_model->index(row, 0, root);
    ui->parameterTreeView->setCurrentIndex(current);
    ui->parameterTreeView->edit(current);

    emit parameterAdded(m_model->data(
                            m_model->index(row, 1, root),
                            Qt::DisplayRole
                        ).toString());
    toggleParameterMoveButtons();
}
