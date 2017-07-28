#include "filedelegate.h"

#include <QLineEdit>

FileDelegate::FileDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{

}

void FileDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QLineEdit *edit = static_cast<QLineEdit*>(editor);
	edit->setText(index.data().toString());
}
