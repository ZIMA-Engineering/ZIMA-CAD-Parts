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
    if (isSelected(dir, partPath))
        return;

    if (m_selected.contains(dir))
        m_selected[dir] << partPath;
    else
        m_selected.insert(dir, QStringList() << partPath);
    emit changed();
}

bool PartSelector::isSelected(const QString &dir, const QString &partPath) const
{
    if (!m_selected.contains(dir))
        return false;

    return m_selected.value(dir).contains(partPath);
}

void PartSelector::clear()
{
    if (m_selected.isEmpty())
        return;
    m_selected.clear();
    emit changed();
}

void PartSelector::clear(const QString &dir)
{
    if (m_selected.remove(dir))
        emit changed();
}

void PartSelector::clear(const QString &dir, const QString &partPath)
{
    if (m_selected[dir].removeOne(partPath))
        emit changed();
}

bool PartSelector::toggle(const QString &dir, const QString &partPath)
{
    if (isSelected(dir, partPath)) {
        clear(dir, partPath);
        return false;
    } else {
        select(dir, partPath);
        return true;
    }
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
