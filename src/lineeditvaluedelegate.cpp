#include "lineeditvaluedelegate.h"

#include <QLineEdit>

LineEditValueDelegate::LineEditValueDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{

}

void LineEditValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = static_cast<QLineEdit*>(editor);
    edit->setText(index.data().toString());
}
