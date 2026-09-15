#ifndef PARTS_UPDATESPAGE_H
#define PARTS_UPDATESPAGE_H
#include <QWidget>
class QCheckBox;
class QLabel;
class QPushButton;
class QProgressBar;
class QPlainTextEdit;
class UpdatesPage : public QWidget
{
    Q_OBJECT
public:
    explicit UpdatesPage(QWidget *parent = nullptr);
    void save();
protected:
    void changeEvent(QEvent *event) override;
private:
    void refresh();
    void retranslate();
    QCheckBox *m_automatic;
    QLabel *m_status, *m_versions, *m_detail;
    QPlainTextEdit *m_notes;
    QProgressBar *m_progress;
    QPushButton *m_check, *m_install, *m_rollback, *m_cancel;
};
#endif
