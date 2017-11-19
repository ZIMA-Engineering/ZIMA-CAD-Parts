#include "partselector.h"

PartSelector* PartSelector::m_instance = nullptr;

PartSelector *PartSelector::get()
{
	if (!m_instance)
		m_instance = new PartSelector;

	return m_instance;
}

void PartSelector::select(const QString &dir, const QString &partPath)
{
	if (m_selected.contains(dir))
		m_selected[dir] << partPath;
	else
		m_selected.insert(dir, QStringList() << partPath);
}

bool PartSelector::isSelected(const QString &dir, const QString &partPath) const
{
	if (!m_selected.contains(dir))
		return false;

	return m_selected.value(dir).contains(partPath);
}

void PartSelector::clear()
{
	m_selected.clear();
}

void PartSelector::clear(const QString &dir)
{
	m_selected.remove(dir);
}

void PartSelector::clear(const QString &dir, const QString &partPath)
{
	m_selected[dir].removeOne(partPath);
}

void PartSelector::toggle(const QString &dir, const QString &partPath)
{
	if (isSelected(dir, partPath))
		clear(dir, partPath);
	else
		select(dir, partPath);
}

QStringList PartSelector::allSelected() const
{
	QStringList ret;
	auto it = allSelectedIterator();

	while (it.hasNext())
	{
		it.next();
		ret << it.value();
	}

	return ret;
}

QHashIterator<QString, QStringList> PartSelector::allSelectedIterator() const
{
	return QHashIterator<QString, QStringList>(m_selected);
}

PartSelector::PartSelector() : QObject()
{

}
