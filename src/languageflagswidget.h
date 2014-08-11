#ifndef LANGUAGEFLAGSWIDGET_H
#define LANGUAGEFLAGSWIDGET_H

#include <QWidget>

class QButtonGroup;


/*! Handle language switching from the GUI.
 * The MetadataCache is reset after language switch.
 * Dependent widgets should register itself to MetadataCache cleared() signal
 */
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
