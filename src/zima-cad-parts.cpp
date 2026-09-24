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

#include <cstdio>
#include <cstring>
#include <QApplication>
#include "zima-cad-parts.h"
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>
#ifdef HAVE_OCCT
#include <QSurfaceFormat>
#endif
#include "browserprofilemanager.h"
#include "mainwindow.h"
#include "settings.h"
#include "update/installationclient.h"
#include <QMessageBox>
#include "applicationlanguage.h"
#include "interactionstyle.h"

/**
\mainpage ZIMA-CAD-Parts Developer Documentation

ZIMA-CAD-Parts is a tool, develop by ZIMA-Engineering (www.zima-engineering.cz),
used by design engineers for management of CAD files.

It can manage local CAD projects with the possibility of connection with PDM system.
It is also a source of library CAD parts. It works with local or remote data storage.
It supports Pro/Engineer, CATIA, NX (UGS), SolidWorks, SolidEdge, Inventor and
neutral formats, i.e. STEP, IGES, DWG, DXF, STL, BLEND and PDF.

The application is written in C++/Qt, it is multiplatform and supports Windows, MAC and Linux.

\section Basic Info

All metadata and thumbnails are stored in special directory
 - 0000-index
 - 0000-index/thumbnails/

See settings.h for its definitions

See metadata.h for metadata handling description

*/

int main(int argc, char *argv[])
{
    // Packaging can verify the binary version without starting the GUI/WebEngine.
    if (argc == 2 && std::strcmp(argv[1], "--build-info") == 0)
    {
        std::printf("{\"version\":\"%s\"}\n", VERSION);
        return 0;
    }

    Q_INIT_RESOURCE(zima_cad_parts);

    QCoreApplication::setOrganizationName("ZIMA-Construction");
    QCoreApplication::setOrganizationDomain("zima-contruction.cz");
    QCoreApplication::setApplicationName("ZIMA-CAD-Parts");
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

#ifdef HAVE_OCCT
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#ifdef Q_OS_WIN
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
#ifdef Q_OS_MAC
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(3, 2);
#endif
    QSurfaceFormat::setDefaultFormat(fmt);
#endif

    QApplication a(argc, argv);
    PartsInteraction::install(a);
    QString instanceError;
    if (!registerPartsInstance(&instanceError)) { QMessageBox::warning(nullptr, "ZIMA-CAD-Parts", instanceError); return 1; }
    QGuiApplication::setDesktopFileName("cz.zima_engineering.ZimaCadParts");
    const QIcon applicationIcon = QIcon::fromTheme(
                "cz.zima_engineering.ZimaCadParts",
                QIcon(":/gfx/icon.png"));
    QApplication::setWindowIcon(applicationIcon);

    applyApplicationLanguage(Settings::get()->getCurrentLanguageCode());
    int ret = 0;

    {
        MainWindow w(nullptr);
        w.showMaximized();

        ret = a.exec();
    }

    BrowserProfileManager::shutdown();
    Settings::get()->save();
    return ret;
}
