#include "settings.h"
#include "zimautils.h"
#include "filefilters/extensionfilter.h"
#include "filefilters/versionfilter.h"
#include "zima-cad-parts.h"

#include <QSettings>
#include <QDir>
#include <QtDebug>


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

    DataSourcesNeedsUpdate = false;

    Languages << "en_US" << "cs_CZ" << "de_DE" << "ru_RU";
    Language = s.value("Language", "detect").toString();
    LanguageMetadata = s.value("LanguageMetadata", "en").toString();

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

    foreach (QVariant i, s.value("Server/SplitterSizes").toList())
        ServersSplitterSizes << i.toInt();

    WorkingDir = s.value("WorkingDir", DEFAULT_WDIR).toString();

    GUIThumbWidth = s.value("GUIThumbWidth", 32).toInt();
    GUIPreviewWidth = s.value("GUIPreviewWidth", 256).toInt();
    GUISplashEnabled = s.value("GUISplashEnabled", true).toBool();
    GUISplashDuration = s.value("GUISplashDuration", 1500).toInt();
    DeveloperEnabled = s.value("DeveloperEnabled", false).toBool();
    DeveloperDirWebViewToolBar = s.value("DeveloperTechSpecToolBar", true).toBool();
    ProeExecutable = s.value("ExternalPrograms/ProE/Executable", "proe.exe").toString();
    TextEditorPath = s.value("ExternalPrograms/TextEditorPath").toString();
    TerminalPath = s.value("ExternalPrograms/TerminalPath").toString();

    MainTabs = s.value("Tabs/Open", QStringList()).toStringList();
    ActiveMainTab = s.value("Tabs/Active", 0).toInt();

    loadDataSources();
    setupFilterGroups();
}

void Settings::save()
{
    QSettings s;

    s.setValue("Language", Language);
    s.setValue("LanguageMetadata", LanguageMetadata);

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

    QVariantList l;
    foreach (int i, ServersSplitterSizes)
    {
        l << i;
    }
    s.setValue("Server/SplitterSizes", l);

    s.setValue("WorkingDir", WorkingDir);

    s.setValue("GUIThumbWidth", GUIThumbWidth);
    s.setValue("GUIPreviewWidth", GUIPreviewWidth);
    s.setValue("GUISplashEnabled", GUISplashEnabled);
    s.setValue("GUISplashDuration", GUISplashDuration);
    s.setValue("DeveloperEnabled", DeveloperEnabled);
    s.setValue("DeveloperTechSpecToolBar", DeveloperDirWebViewToolBar);
    s.setValue("ExternalPrograms/ProE/Executable", ProeExecutable);
    s.setValue("ExternalPrograms/TextEditorPath", TextEditorPath);
    s.setValue("ExternalPrograms/TerminalPath", TerminalPath);

    saveDataSources();
    saveFilters();
}

void Settings::loadDataSources()
{
    QSettings settings;
    //qDebug(settings.fileName().toLocal8Bit().constData());

    settings.beginGroup("DataSources");
    foreach(QString str, settings.childGroups())
    {
        settings.beginGroup(str);

        QString name = settings.value("Label", QString()).toString();
        QString path = settings.value("Path", QString()).toString();
        if (path.isNull())
        {
            qDebug() << "Datasource" << name << "has an empty path. Skipping.";
            continue;
        }

        DataSource *ds = new DataSource(name, path);
        DataSources.append(ds);

        settings.endGroup();
    }
    settings.endGroup();

    qDebug() << "All datasources count:" << DataSources.size();
}


void Settings::saveDataSources()
{
    int i = 0;
    QSettings settings;

    settings.remove("DataSources");
    settings.beginGroup("DataSources");
    foreach(DataSource *ds, DataSources)
    {
        settings.beginGroup(QString::number(i++));

        settings.setValue("Label", ds->name);
        settings.setValue("Path", ds->rootPath);
        settings.endGroup();
    }
    settings.endGroup();
}

void Settings::setupFilterGroups()
{
    FilterGroups << FilterGroup("ProE", "Pro/Engineer");
    FilterGroups.last()
            << new ExtensionFilter(FileType::PROE_PRT)
            << new ExtensionFilter(FileType::PROE_ASM)
            << new ExtensionFilter(FileType::PROE_DRW)
            << new ExtensionFilter(FileType::FRM)
            << new ExtensionFilter(FileType::PROE_NEU)
            << new VersionFilter();

    FilterGroups << FilterGroup("CATIA", "CATIA");
    FilterGroups.last()
            << new ExtensionFilter(FileType::CATPART)
            << new ExtensionFilter(FileType::CATPRODUCT)
            << new ExtensionFilter(FileType::CATDRAWING);

    FilterGroups << FilterGroup("NX", "NX (UGS)");
    FilterGroups.last()
            << new ExtensionFilter(FileType::NX_PRT);

    FilterGroups << FilterGroup("SolidWorks", "SolidWorks");
    FilterGroups.last().filters
            << new ExtensionFilter(FileType::SLDPRT)
            << new ExtensionFilter(FileType::SLDASM)
            << new ExtensionFilter(FileType::SLDDRW);

    FilterGroups << FilterGroup("SolidEdge", "Solid Edge");
    FilterGroups.last()
            << new ExtensionFilter(FileType::PAR)
            << new ExtensionFilter(FileType::PSM)
            << new ExtensionFilter(FileType::PROE_ASM)
            << new ExtensionFilter(FileType::DFT);

    FilterGroups << FilterGroup("Invertor", "INVERTOR");
    FilterGroups.last()
            << new ExtensionFilter(FileType::IPT)
            << new ExtensionFilter(FileType::IAM)
            << new ExtensionFilter(FileType::IDW);

    FilterGroups << FilterGroup("VariCAD", "VariCAD");
    FilterGroups.last()
            << new ExtensionFilter(FileType::DWB);

    FilterGroups << FilterGroup("FreeCAD", "FreeCAD");
    FilterGroups.last()
            << new ExtensionFilter(FileType::FCSTD);

    FilterGroups << FilterGroup("CADNeutral", "CAD NEUTRAL");
    FilterGroups.last()
            << new ExtensionFilter(FileType::STEP)
            << new ExtensionFilter(FileType::IGES)
            << new ExtensionFilter(FileType::DWG)
            << new ExtensionFilter(FileType::DXF);

    FilterGroups << FilterGroup("Archives", "Archives");
    FilterGroups.last()
            << new ExtensionFilter(FileType::ZIP)
            << new ExtensionFilter(FileType::RAR)
            << new ExtensionFilter(FileType::TAR)
            << new ExtensionFilter(FileType::ZIP7);

    FilterGroups << FilterGroup("NonCAD", "NonCAD");
    FilterGroups.last()
            << new ExtensionFilter(FileType::STL)
            << new ExtensionFilter(FileType::BLEND)
            << new ExtensionFilter(FileType::PDF)
            << new ExtensionFilter(FileType::OFFICE_BASE)
            << new ExtensionFilter(FileType::OFFICE_CALC)
            << new ExtensionFilter(FileType::OFFICE_DRAW)
            << new ExtensionFilter(FileType::OFFICE_IMPRESS)
            << new ExtensionFilter(FileType::OFFICE_PROJECT)
            << new ExtensionFilter(FileType::OFFICE_WRITER)
            << new ExtensionFilter(FileType::OFFICE_EML)
            << new ExtensionFilter(FileType::OFFICE_MBOX)
            << new ExtensionFilter(FileType::FILE_IMAGE)
            << new ExtensionFilter(FileType::FILE_AUDIO);

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

    filtersRegex = QRegularExpression("^" + expressions.join("|") + "$", QRegularExpression::CaseInsensitiveOption);
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
    else if ( lang == "de_DE" )
        return GERMAN;
    else if ( lang == "ru_RU" )
        return RUSSIAN;
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
    case GERMAN:
        return "de_DE";
    case RUSSIAN:
        return "ru_RU";
    default:
        return "detect";
    }
}

void Settings::setMainTabs(QStringList tabs, int current)
{
    QSettings s;
    s.setValue("Tabs/Open", tabs);
    s.setValue("Tabs/Active", current);

    MainTabs = tabs;
    ActiveMainTab = current;
}

void Settings::setWorkingDir(const QString &wd)
{
    WorkingDir = wd;

    QSettings s;
    s.setValue("WorkingDir", WorkingDir);
    s.sync();
}
