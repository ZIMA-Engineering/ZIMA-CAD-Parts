// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef AISETTINGSPAGE_H
#define AISETTINGSPAGE_H
#include <QWidget>
class AiProvider;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class AiSettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit AiSettingsPage(QWidget *parent = nullptr);
    void save();
protected:
    void changeEvent(QEvent *event) override;
private:
    void refresh();
    void retranslate();
    AiProvider *m_provider;
    QLineEdit *m_executable;
    QComboBox *m_model;
    QPushButton *m_browse, *m_connect, *m_login, *m_logout, *m_cancel, *m_help;
    QLabel *m_intro, *m_pathLabel, *m_modelLabel, *m_status;
};
#endif
