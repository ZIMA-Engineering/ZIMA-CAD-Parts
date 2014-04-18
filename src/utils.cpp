#include "utils.h"

QList<FilterGroup> Utils::filterGroups;


QList<BaseDataSource*> Utils::loadDataSources()
{
    QList<BaseDataSource*> servers;
    QSettings settings;

    settings.beginGroup("DataSources");
    foreach(QString str, settings.childGroups())
    {
        settings.beginGroup(str);

        QString dataSourceType = settings.value("DataSourceType", "ftp").toString();

        if( dataSourceType == "ftp" )
        {
            FtpDataSource *s = new FtpDataSource();
            s->loadSettings(*settings);
            servers.append(s);
        } else if ( dataSourceType == "local" ) {
            LocalDataSource *s = new LocalDataSource();
            s->loadSettings(*settings);
            servers.append(s);
        }

        settings.endGroup();
    }
    settings.endGroup();

    return servers;
}


void Utils::saveDataSources(const QList<BaseDataSource*> &data)
{
    int i = 0;

    settings.remove("DataSources");
    settings.beginGroup("DataSources");
    foreach(BaseDataSource *bs, data)
    {
        settings.beginGroup(QString::number(i++));
        switch( bs->dataSource )
        {
        case LOCAL: {
            LocalDataSource *s = static_cast<LocalDataSource*>(bs);

            s->saveSettings(*settings);
            break;
        }
        case FTP: {
            FtpDataSource *s = static_cast<FtpDataSource*>(bs);

            s->saveSettings(*settings);
            break;
        }
        default:
            break;
        }

        settings.endGroup();
    }
    settings.endGroup();
}

void Utils::setupFilterGroups()
{
    filterGroups << FilterGroup("ProE", "Pro/Engineer");
    filterGroups.last()
            << new ExtensionFilter(File::PRT_PROE)
            << new ExtensionFilter(File::ASM)
            << new ExtensionFilter(File::DRW)
            << new ExtensionFilter(File::FRM)
            << new ExtensionFilter(File::NEU_PROE)
            << new VersionFilter();

    filterGroups << FilterGroup("CATIA", "CATIA");
    filterGroups.last()
            << new ExtensionFilter(File::CATPART)
            << new ExtensionFilter(File::CATPRODUCT)
            << new ExtensionFilter(File::CATDRAWING);

    filterGroups << FilterGroup("NX", "NX (UGS)");
    filterGroups.last()
            << new ExtensionFilter(File::PRT_NX);

    filterGroups << FilterGroup("SolidWorks", "SolidWorks");
    filterGroups.last().filters
            << new ExtensionFilter(File::SLDPRT)
            << new ExtensionFilter(File::SLDASM)
            << new ExtensionFilter(File::SLDDRW);

    filterGroups << FilterGroup("SolidEdge", "Solid Edge");
    filterGroups.last()
            << new ExtensionFilter(File::PAR)
            << new ExtensionFilter(File::PSM)
            << new ExtensionFilter(File::ASM)
            << new ExtensionFilter(File::DFT);

    filterGroups << FilterGroup("Invertor", "INVERTOR");
    filterGroups.last()
            << new ExtensionFilter(File::IPT)
            << new ExtensionFilter(File::IAM)
            << new ExtensionFilter(File::IDW);

    filterGroups << FilterGroup("CADNeutral", "CAD NEUTRAL");
    filterGroups.last()
            << new ExtensionFilter(File::STEP)
            << new ExtensionFilter(File::IGES)
            << new ExtensionFilter(File::DWG)
            << new ExtensionFilter(File::DXF);

    filterGroups << FilterGroup("NonCAD", "NonCAD");
    filterGroups.last()
            << new ExtensionFilter(File::STL)
            << new ExtensionFilter(File::BLEND)
            << new ExtensionFilter(File::PDF)
            << new ExtensionFilter(File::OFFICE_BASE)
            << new ExtensionFilter(File::OFFICE_CALC)
            << new ExtensionFilter(File::OFFICE_DRAW)
            << new ExtensionFilter(File::OFFICE_IMPRESS)
            << new ExtensionFilter(File::OFFICE_PROJECT)
            << new ExtensionFilter(File::OFFICE_WRITER);

    QSettings settings;
    settings.beginGroup("PartFilters");

    int cnt = filterGroups.count();

    for(int i = 0; i < cnt; i++)
    {
        settings.beginGroup(filterGroups[i].internalName);
        filterGroups[i].enabled = settings.value("Enabled", true).toBool();

        int filterCnt = filterGroups[i].filters.count();

        for(int j = 0; j < filterCnt; j++)
            filterGroups[i].filters[j]->load(settings);

        settings.endGroup();
    }

    settings.endGroup();
}

void Utils::saveFilters()
{
    QSettings settings;
    settings.beginGroup("PartFilters");

    int cnt = filterGroups.count();

    for(int i = 0; i < cnt; i++)
    {
        settings.beginGroup(filterGroups[i].internalName);
        settings.setValue("Enabled", filterGroups[i].enabled);

        int filterCnt = filterGroups[i].filters.count();

        for(int j = 0; j < filterCnt; j++)
            filterGroups[i].filters[j]->save(settings);

        settings.endGroup();
    }

    settings.endGroup();
}
