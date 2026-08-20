#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include <QDialog>

#include "filefilterconfig.h"

class QTreeWidgetItem;

namespace Ui { class FiltersDialog; }

class FiltersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FiltersDialog(const QString &directory, QWidget *parent = nullptr);
    ~FiltersDialog();
    void accept() override;

private slots:
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void treeItemChanged(QTreeWidgetItem *item, int column);
    void updateCurrentItem();
    void addGroup();
    void addFormat();
    void deleteCurrent();
    void moveCurrentUp();
    void moveCurrentDown();

private:
    enum ItemRole { GroupIndexRole = Qt::UserRole, FormatIndexRole };

    Ui::FiltersDialog *ui;
    QString m_directory;
    FileFilterConfig m_config;
    bool m_updating;

    void setupDefaults();
    void rebuildTree(int groupIndex = 0, int formatIndex = -1);
    void commitEditor();
    QString uniqueGroupId(const QString &base) const;
};

#endif // FILTERSDIALOG_H
