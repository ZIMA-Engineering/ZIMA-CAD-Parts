#ifndef UTILS_H
#define UTILS_H

#include "filefilters/filtergroup.h"

class Utils
{
public:

    static QList<FilterGroup> filterGroups;

    static QList<BaseDataSource*> loadDataSources();
    static void saveDataSources(const QList<BaseDataSource*> &data);
    static void setupFilterGroups();

    static void saveFilters();
};

#endif // UTILS_H
