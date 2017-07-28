#ifndef FILEDELEGATE_H
#define FILEDELEGATE_H

#include <QStyledItemDelegate>

/*! Delegate for editing part metadata.
 *
 * The purpose of this delegate is to provide the editing widget with
 * the current value, so that the user does not have to type it anew,
 * if he only wants to fix some typo.
 */
class FileDelegate : public QStyledItemDelegate
{
public:
	FileDelegate(QWidget *parent = 0);
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

};

#endif // FILEDELEGATE_H
