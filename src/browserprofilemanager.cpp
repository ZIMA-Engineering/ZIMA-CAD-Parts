/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "browserprofilemanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

BrowserProfileManager *BrowserProfileManager::instance()
{
    static BrowserProfileManager instance(QCoreApplication::instance());
    return &instance;
}

BrowserProfileManager::BrowserProfileManager(QObject *parent)
    : QObject(parent),
      m_profile(0)
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + "/browser-profile";
    const QString storagePath = basePath + "/storage";
    const QString cachePath = basePath + "/cache";

    QDir().mkpath(storagePath);
    QDir().mkpath(cachePath);

    m_profile = new QWebEngineProfile(QStringLiteral("Default"), this);
    m_profile->setPersistentStoragePath(storagePath);
    m_profile->setCachePath(cachePath);
    m_profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

    m_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    m_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

    connect(m_profile, SIGNAL(downloadRequested(QWebEngineDownloadRequest*)),
            this, SIGNAL(downloadRequested(QWebEngineDownloadRequest*)));
}

QWebEngineProfile *BrowserProfileManager::profile() const
{
    return m_profile;
}
