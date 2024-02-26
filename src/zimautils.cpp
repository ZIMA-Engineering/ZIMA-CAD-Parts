#include "zimautils.h"
#include "settings.h"


QString ZimaUtils::internalNameForUtility(int util)
{
    return internalNameForUtility((ZimaUtility) util);
}

QString ZimaUtils::internalNameForUtility(ZimaUtility util)
{
    switch(util)
    {
    case ZimaPtcCleaner:
        return "ZimaPtcCleaner";
    case ZimaCadSync:
        return "ZimaCadSync";
    case ZimaPs2Pdf:
        return "ZimaPs2Pdf";
    case ZimaStepEdit:
        return "ZimaStepEdit";
    default:
        return "invalid";
    }
}

QString ZimaUtils::labelForUtility(int util)
{
    return labelForUtility((ZimaUtility) util);
}

QString ZimaUtils::labelForUtility(ZimaUtility util)
{
    switch(util)
    {
    case ZimaPtcCleaner:
        return "ZIMA-PTC-Cleaner";
    case ZimaCadSync:
        return "ZIMA-CAD-Sync";
    case ZimaPs2Pdf:
        return "ZIMA-PS2PDF";
    case ZimaStepEdit:
        return "ZIMA-STEP-Edit";
    default:
        return "invalid";
    }
}

QHash<QString,QString> ZimaUtils::paths()
{
    return Settings::get()->ExternalPrograms;
}
