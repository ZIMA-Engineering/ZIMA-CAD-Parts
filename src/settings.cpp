#include "settings.h"
#include "zimautils.h"

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
    HomeDir = s.value("HomeDir").toString();
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
    s.setValue("HomeDir", HomeDir);
}
