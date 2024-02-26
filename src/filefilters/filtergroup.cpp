#include <QFont>

#include "filtergroup.h"

FilterGroup& FilterGroup::operator<<(FileFilter *f)
{
    filters << f;
    f->group = this;

    return *this;
}

QTreeWidgetItem* FilterGroup::widget()
{
    item = new QTreeWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setText(0, label);
    item->setCheckState(0, enabled ? Qt::Checked : Qt::Unchecked);

    QFont f = item->font(0);
    f.setBold(true);
    item->setFont(0, f);

    int filterCnt = filters.count();

    for(int j = 0; j < filterCnt; j++)
        item->addChild(filters[j]->widget());

    return item;
}

void FilterGroup::apply()
{
    enabled = item->checkState(0) == Qt::Checked;

    int filterCnt = filters.count();

    for(int j = 0; j < filterCnt; j++)
        filters[j]->apply();

    delete item;
}
