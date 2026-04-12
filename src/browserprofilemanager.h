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

#ifndef BROWSERPROFILEMANAGER_H
#define BROWSERPROFILEMANAGER_H

#include <QObject>

class QWebEngineDownloadRequest;
class QWebEngineProfile;
class PasswordManager;

class BrowserProfileManager : public QObject
{
    Q_OBJECT

public:
    static BrowserProfileManager *instance();

    QWebEngineProfile *profile() const;
    PasswordManager *passwordManager() const;

signals:
    void downloadRequested(QWebEngineDownloadRequest *download);

private:
    explicit BrowserProfileManager(QObject *parent = 0);

    QWebEngineProfile *m_profile;
    PasswordManager *m_passwordManager;
};

#endif // BROWSERPROFILEMANAGER_H
