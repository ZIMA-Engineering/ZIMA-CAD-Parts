#ifndef SETTINGS_H
#define SETTINGS_H

#include <QHash>


class Settings
{
public:
    static Settings *get();

    void save();

    // real settings

    QHash<QString,QString> ExternalPrograms;
    QString Language;
    QString WorkingDir;
    QString HomeDir;
    QByteArray MainWindowState;
    QByteArray MainWindowGeometry;

private:
    // Singleton handling
    static Settings *m_instance;

    Settings();
    Settings(const Settings &) {};
    ~Settings();

    void load();
};

#endif // SETTINGS_H
