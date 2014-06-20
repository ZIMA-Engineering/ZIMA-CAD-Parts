#ifndef SETTINGS_H
#define SETTINGS_H

#include <QHash>
#include "basedatasource.h"
#include "filefilters/filtergroup.h"


class Settings
{
public:
	enum Languages {
		DETECT=0,
		ENGLISH,
		CZECH
	};

	static Settings *get();

	void save();
	QString getCurrentLanguageCode();
	void setCurrentLanguageCode(const QString &lang);

	// real settings

	QHash<QString,QString> ExternalPrograms;
	QString WorkingDir;
	QByteArray MainWindowState;
	QByteArray MainWindowGeometry;

	QList<int> ServersSplitterSizes;

	int GUIThumbWidth;
	int GUIPreviewWidth;
	bool GUISplashEnabled;
	int GUISplashDuration;
	bool DeveloperEnabled;
	bool DeveloperTechSpecToolBar;

	QString ExtensionsProductViewPath;
	QByteArray ExtensionsProductViewGeometry;
	QPoint ExtensionsProductViewPosition;

	QString ProeExecutable;

	bool DataSourcesNeedsUpdate;
	DataSourceList DataSources;
	QList<FilterGroup> FilterGroups;

	QRegExp filtersRegex;
	bool ShowProeVersions;
	void recalculateFilters();

	QStringList Languages;

	int langIndex(const QString &lang);
	QString langIndexToName(int lang);

private:
	// cannot access it directly. Use getCurrentLanguageCode()
	QString Language;

	// Singleton handling
	static Settings *m_instance;

	Settings();
	Settings(const Settings &) {};
	~Settings();

	void load();
	void loadDataSources();
	void saveDataSources();
	void setupFilterGroups();
	void saveFilters();
};

#endif // SETTINGS_H
