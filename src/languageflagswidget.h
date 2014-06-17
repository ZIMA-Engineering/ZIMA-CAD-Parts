#ifndef LANGUAGEFLAGSWIDGET_H
#define LANGUAGEFLAGSWIDGET_H

#include <QWidget>

class QButtonGroup;


class LanguageFlagsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LanguageFlagsWidget(QWidget *parent = 0);

signals:
    void changeLanguage(int);

public slots:

private:
    QButtonGroup *m_buttons;
};

#endif // LANGUAGEFLAGSWIDGET_H
