#include "settings.h"
#include "zimautils.h"
#include "localdatasource.h"
#include "ftpdatasource.h"
#include "filefilters/extensionfilter.h"
#include "filefilters/versionfilter.h"
#include "zima-cad-parts.h"

#include <QSettings>
#include <QDir>


Settings * Settings::m_instance = 0;


Settings * Settings::get()
{
    if (!m_instance)
        m_instance = new Settings();
    return m_instance;
}

Settings::Settings()
{
    load();
}

Settings::~Settings()
{
    save();
    delete m_instance;
    m_instance = 0;
}

void Settings::load()
{
    QSettings s;

    DataSourcesNeedsUpdate = true;

    Languages << "en_US" << "cs_CZ" << "de_DE" << "ru_RU";
    Language = s.value("Language", "detect").toString();

    // zima utils
    s.beginGroup("ExternalPrograms");
    QString key;
    for(int i = 0; i < ZimaUtils::ZimaUtilsCount; i++)
    {
        key = ZimaUtils::internalNameForUtility(i);
        s.beginGroup(key);
        ExternalPrograms[key] = s.value("Executable").toString();
        s.endGroup();
    }
    s.endGroup();

    MainWindowState = s.value("state").toByteArray();
    MainWindowGeometry = s.value("geometry").toByteArray();
    WorkingDir = s.value("WorkingDir", QDir::homePath() + "/ZIMA-CAD-Parts").toString();

    GUIThumbWidth = s.value("GUI/ThumbWidth", 32).toInt();
    GUIPreviewWidth = s.value("GUI/PreviewWidth", 256).toInt();
    GUISplashEnabled = s.value("GUI/Splash/Enabled", true).toBool();
    GUISplashDuration = s.value("GUI/Splash/Duration", 1500).toInt();
    DeveloperEnabled = s.value("Developer/Enabled", false).toBool();
    DeveloperTechSpecToolBar = s.value("Developer/TechSpecToolBar", true).toBool();
    ExtensionsProductViewPath = s.value("Extensions/ProductView/Path", PRODUCT_VIEW_DEFAULT_PATH).toString();
    ExtensionsProductViewGeometry = s.value("Extensions/ProductView/geometry").toByteArray();
    ExtensionsProductViewPosition = s.value("Extensions/ProductView/position").toPoint();
    ProeExecutable = s.value("ExternalPrograms/ProE/Executable", "proe.exe").toString();

    loadDataSources();
    setupFilterGroups();
}

void Settings::save()
{
    QSettings s;

    s.setValue("Language", Language);

    // zima utils
    s.beginGroup("ExternalPrograms");
    QHashIterator<QString,QString> it(ExternalPrograms);
    while (it.hasNext())
    {
        it.next();
        s.beginGroup(it.key());
        s.setValue("Executable", it.value());
        s.endGroup();
    }
    s.endGroup();

    s.setValue("state", MainWindowState);
    s.setValue("geometry", MainWindowGeometry);
    s.setValue("WorkingDir", WorkingDir);

    s.setValue("GUIThumbWidth", GUIThumbWidth);
    s.setValue("GUIPreviewWidth", GUIPreviewWidth);
    s.setValue("GUISplashEnabled", GUISplashEnabled);
    s.setValue("GUISplashDuration", GUISplashDuration);
    s.setValue("DeveloperEnabled", DeveloperEnabled);
    s.setValue("DeveloperTechSpecToolBar", DeveloperTechSpecToolBar);
    s.setValue("ExtensionsProductViewPath", ExtensionsProductViewPath);
    s.setValue("ExternalPrograms/ProE/Executable", ProeExecutable);

    saveDataSources();
    saveFilters();
}

void Settings::loadDataSources()
{

    QSettings settings;

    settings.beginGroup("DataSources");
    foreach(QString str, settings.childGroups())
    {
        settings.beginGroup(str);

        QString dataSourceType = settings.value("DataSourceType", "ftp").toString();
        BaseDataSource *ds = 0;

        if( dataSourceType == "ftp" )
        {
            FtpDataSource *ftpds = new FtpDataSource();
            ds = ftpds;

            ftpds->remoteHost = settings.value("Host", "localhost").toString();
            ftpds->remotePort = settings.value("Port", "21").toInt();
            //ftpds->remoteBaseDir = settings.value("BaseDir", "/").toString();
            ftpds->remoteLogin = settings.value("Login", "").toString();
            ftpds->remotePassword = settings.value("Password", "").toString();
            ftpds->ftpPassiveMode = settings.value("PassiveMode", true).toBool();

        } else if ( dataSourceType == "local" ) {
            LocalDataSource *locds = new LocalDataSource();
            ds = locds;
            locds->localPath = settings.value("Path", "").toString();
        }

        ds->label = settings.value("Label").toString();
        DataSources.append(ds);

        settings.endGroup();
    }
    settings.endGroup();
}


void Settings::saveDataSources()
{
    int i = 0;
    QSettings settings;

    settings.remove("DataSources");
    settings.beginGroup("DataSources");
    foreach(BaseDataSource *ds, DataSources)
    {
        settings.beginGroup(QString::number(i++));

        settings.setValue("Label", ds->label);
        settings.setValue("DataSourceType", ds->internalName());

        switch( ds->dataSource )
        {
        case LOCAL: {
            LocalDataSource *locds = static_cast<LocalDataSource*>(ds);
            settings.setValue("Path", locds->localPath);
            break;
        }
        case FTP: {
            FtpDataSource *ftpds = static_cast<FtpDataSource*>(ds);
            settings.setValue("Host", ftpds->remoteHost);
            settings.setValue("Port", ftpds->remotePort);
            //settings.setValue("BaseDir", ftpds->remoteBaseDir);
            settings.setValue("Login", ftpds->remoteLogin);
            settings.setValue("Password", ftpds->remotePassword);
            settings.setValue("PassiveMode", ftpds->ftpPassiveMode);
            break;
        }
        default:
            break;
        }

        settings.endGroup();
    }
    settings.endGroup();
}

void Settings::setupFilterGroups()
{
    FilterGroups << FilterGroup("ProE", "Pro/Engineer");
    FilterGroups.last()
            << new ExtensionFilter(File::PRT_PROE)
            << new ExtensionFilter(File::ASM)
            << new ExtensionFilter(File::DRW)
            << new ExtensionFilter(File::FRM)
            << new ExtensionFilter(File::NEU_PROE)
            << new VersionFilter();

    FilterGroups << FilterGroup("CATIA", "CATIA");
    FilterGroups.last()
            << new ExtensionFilter(File::CATPART)
            << new ExtensionFilter(File::CATPRODUCT)
            << new ExtensionFilter(File::CATDRAWING);

    FilterGroups << FilterGroup("NX", "NX (UGS)");
    FilterGroups.last()
            << new ExtensionFilter(File::PRT_NX);

    FilterGroups << FilterGroup("SolidWorks", "SolidWorks");
    FilterGroups.last().filters
            << new ExtensionFilter(File::SLDPRT)
            << new ExtensionFilter(File::SLDASM)
            << new ExtensionFilter(File::SLDDRW);

    FilterGroups << FilterGroup("SolidEdge", "Solid Edge");
    FilterGroups.last()
            << new ExtensionFilter(File::PAR)
            << new ExtensionFilter(File::PSM)
            << new ExtensionFilter(File::ASM)
            << new ExtensionFilter(File::DFT);

    FilterGroups << FilterGroup("Invertor", "INVERTOR");
    FilterGroups.last()
            << new ExtensionFilter(File::IPT)
            << new ExtensionFilter(File::IAM)
            << new ExtensionFilter(File::IDW);

    FilterGroups << FilterGroup("CADNeutral", "CAD NEUTRAL");
    FilterGroups.last()
            << new ExtensionFilter(File::STEP)
            << new ExtensionFilter(File::IGES)
            << new ExtensionFilter(File::DWG)
            << new ExtensionFilter(File::DXF);

    FilterGroups << FilterGroup("NonCAD", "NonCAD");
    FilterGroups.last()
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

    int cnt = FilterGroups.count();

    for(int i = 0; i < cnt; i++)
    {
        settings.beginGroup(FilterGroups[i].internalName);
        FilterGroups[i].enabled = settings.value("Enabled", true).toBool();

        int filterCnt = FilterGroups[i].filters.count();

        for(int j = 0; j < filterCnt; j++)
            FilterGroups[i].filters[j]->load(&settings);

        settings.endGroup();
    }

    settings.endGroup();

    recalculateFilters();
}

void Settings::saveFilters()
{
    QSettings settings;
    settings.beginGroup("PartFilters");

    int cnt = FilterGroups.count();

    for(int i = 0; i < cnt; i++)
    {
        settings.beginGroup(FilterGroups[i].internalName);
        settings.setValue("Enabled", FilterGroups[i].enabled);

        int filterCnt = FilterGroups[i].filters.count();

        for(int j = 0; j < filterCnt; j++)
            FilterGroups[i].filters[j]->save(&settings);

        settings.endGroup();
    }

    settings.endGroup();
}

void Settings::recalculateFilters()
{
    QStringList expressions;

    int cnt = FilterGroups.count();

    for(int i = 0; i < cnt; i++)
    {
        if (!FilterGroups[i].enabled)
            continue;

        int filterCnt = FilterGroups[i].filters.count();

        for(int j = 0; j < filterCnt; j++)
        {
            switch(FilterGroups[i].filters[j]->filterType())
            {
            case FileFilter::Extension:
                if(FilterGroups[i].filters[j]->enabled)
                    expressions << File::getRxForFileType(FilterGroups[i].filters[j]->type);
                break;

            case FileFilter::Version:
                ShowProeVersions = FilterGroups[i].filters[j]->enabled;
                break;
            }
        }
    }

    expressions.removeDuplicates();

    filtersRegex = QRegExp( "^" + expressions.join("|") + "$" );
}

QString Settings::getCurrentLanguageCode()
{
    QString lang = Settings::get()->Language;
    return (lang.isEmpty() || lang == "detect") ? QLocale::system().name() : lang;
}

void Settings::setCurrentLanguageCode(const QString &lang)
{
    Language = lang;
}

int Settings::langIndex(const QString &lang)
{
    if( lang.startsWith("en_") )
        return ENGLISH;
    else if( lang == "cs_CZ" )
        return CZECH;
    else
        return DETECT;
}

QString Settings::langIndexToName(int lang)
{
    switch(lang)
    {
    case ENGLISH:
        return "en_US";
    case CZECH:
        return "cs_CZ";
    default:
        return "detect";
    }
}
