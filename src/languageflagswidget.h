#ifndef LANGUAGEFLAGSWIDGET_H
#define LANGUAGEFLAGSWIDGET_H

#include <QWidget>

class QButtonGroup;


class LanguageFlagsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit LanguageFlagsWidget(QWidget *parent = 0);

private:
	QButtonGroup *m_buttons;

private slots:
    void changeLanguage(int langIndex);
};

#endif // LANGUAGEFLAGSWIDGET_H
