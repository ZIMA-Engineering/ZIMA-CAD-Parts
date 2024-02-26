#ifndef LINEEDITVALUEDELEGATE_H
#define LINEEDITVALUEDELEGATE_H

#include <QStyledItemDelegate>

/*! Delegate for editing text values.
 *
 * The purpose of this delegate is to provide the editing widget with
 * the current value, so that the user does not have to type it anew,
 * if he only wants to fix some typo.
 */
class LineEditValueDelegate : public QStyledItemDelegate
{
public:
    LineEditValueDelegate(QWidget *parent = 0);
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

};

#endif // LINEEDITVALUEDELEGATE_H
