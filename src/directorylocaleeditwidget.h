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
	explicit DirectoryLocaleEditWidget(Metadata *meta, const QString &lang, QWidget *parent = 0);
	~DirectoryLocaleEditWidget();
	QString label() const;

public slots:
	void apply(Metadata *meta);
	void addParameter(const QString &handle);
	void parameterHandleChange(const QString &handle, const QString &newHandle);
	void removeSelectedParameter();
	void parameterSelected(const QModelIndex &current, const QModelIndex &previous);
	void removeParameter(const QString &handle);

signals:
	void parameterAdded(const QString &handle);
	void parameterHandleChanged(const QString &handle, const QString &newHandle);
	void parameterRemoved(const QString &handle);

private:
	Ui::DirectoryLocaleEditWidget *ui;
	QString m_lang;

private slots:
	void addNewParameter();
};

#endif // DIRECTORYLOCALEEDITWIDGET_H
