#include <QVBoxLayout>

#include "filtergroup.h"

FilterGroup& FilterGroup::operator<<(FileFilter *f)
{
	filters << f;
	f->group = this;

	return *this;
}

QWidget* FilterGroup::widget()
{
	groupBox = new QGroupBox();
	groupBox->setTitle(label);
	groupBox->setCheckable(true);
	groupBox->setChecked(enabled);

	QVBoxLayout *groupLayout = new QVBoxLayout;

	int filterCnt = filters.count();

	for(int j = 0; j < filterCnt; j++)
		groupLayout->addWidget(filters[j]->widget());

	groupLayout->addStretch(100);

	groupBox->setLayout(groupLayout);

	return groupBox;
}

void FilterGroup::apply()
{
	enabled = groupBox->isChecked();

	int filterCnt = filters.count();

	for(int j = 0; j < filterCnt; j++)
		filters[j]->apply();

	delete groupBox;
}
