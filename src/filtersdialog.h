#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H
#include <QDialog>
class QPlainTextEdit;
class QCheckBox;

class FiltersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FiltersDialog(const QString &directory, QWidget *parent = nullptr);
    void accept() override;
private:
    QString m_directory;
    QPlainTextEdit *m_hidden;
    QCheckBox *m_versions;
    QCheckBox *m_zimaVersions;
};
#endif
