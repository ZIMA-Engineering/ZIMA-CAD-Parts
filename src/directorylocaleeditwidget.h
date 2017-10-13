#ifndef DIRECTORYLOCALEEDITWIDGET_H
#define DIRECTORYLOCALEEDITWIDGET_H

#include <QWidget>

class Metadata;

namespace Ui {
class DirectoryLocaleEditWidget;
}

class DirectoryLocaleEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, const QStringList &primaryColumns, QWidget *parent = 0);
	~DirectoryLocaleEditWidget();
	QString label() const;

public slots:
	void apply(Metadata *meta);
	void addPrimaryColumn(int row);

signals:
	void primaryColumnAdded(int row);

private:
	Ui::DirectoryLocaleEditWidget *ui;
	QString m_lang;

private slots:
	void addNewColumn();
};

#endif // DIRECTORYLOCALEEDITWIDGET_H
