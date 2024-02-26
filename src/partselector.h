#ifndef PARTSELECTOR_H
#define PARTSELECTOR_H

#include <QObject>
#include <QHash>

class PartSelector : public QObject
{
    Q_OBJECT

public:
    static PartSelector *get();
    void select(const QString &dir, const QString &partPath);
    bool isSelected(const QString &dir, const QString &partPath) const;
    void clear();
    void clear(const QString &dir);
    void clear(const QString &dir, const QString &partPath);
    void toggle(const QString &dir, const QString &partPath);
    QStringList allSelected() const;
    QHashIterator<QString, QStringList> allSelectedIterator() const;

private:
    static PartSelector *m_instance;
    QHash<QString, QStringList> m_selected;

    PartSelector();
};

#endif // PARTSELECTOR_H
