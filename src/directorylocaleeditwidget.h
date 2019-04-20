#ifndef DIRECTORYLOCALEEDITWIDGET_H
#define DIRECTORYLOCALEEDITWIDGET_H

#include <QWidget>

class Metadata;
class DirectoryEditParametersModel;

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
	void reorderParameters(const QStringList &parameters);
	void moveSelectedParameterUp();
	void moveSelectedParameterDown();

signals:
	void parameterAdded(const QString &handle);
	void parameterHandleChanged(const QString &handle, const QString &newHandle);
	void parameterRemoved(const QString &handle);
	void parametersReordered(const QStringList &parameters);

private:
	Ui::DirectoryLocaleEditWidget *ui;
	QString m_lang;
	DirectoryEditParametersModel *m_model;

	void toggleParameterMoveButtons();

private slots:
	void addNewParameter();
};

#endif // DIRECTORYLOCALEEDITWIDGET_H
