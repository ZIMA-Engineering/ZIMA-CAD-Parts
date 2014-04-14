#ifndef UTILS_H
#define UTILS_H

class Utils
{
public:

    static QList<FilterGroup> filterGroups;

    static QList<BaseDataSource*> loadDataSources();
    static void saveDataSources(const QList<BaseDataSource*> &data);
    static void setupFilterGroups();
};

#endif // UTILS_H
