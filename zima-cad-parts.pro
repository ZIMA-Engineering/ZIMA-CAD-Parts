# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network opengl widgets webengine webenginewidgets
TARGET = ZIMA-CAD-Parts
TEMPLATE = app

win32:INCLUDEPATH += ../
VPATH += ./src

INCLUDEPATH += src
INCLUDEPATH += src/filefilters
INCLUDEPATH += libqdxf/src
INCLUDEPATH += libqdxf/libdxfrw/src

# disabled 20140206 by Vlad's request:
# only "supported" files should be displayed. For rest of files this dialog should be closed
#    win32 {
#        INCLUDEPATH += win32/poppler-0.24.5-win32/include
#        LIBS += -L$$PWD/win32/poppler-0.24.5-win32/bin
#        LIBS += -lpoppler-qt5
#    }
#    unix {
#        CONFIG += link_pkgconfig
#        PKGCONFIG += poppler-qt5
#    }

SOURCES += src/zima-cad-parts.cpp \
    src/mainwindow.cpp \
    src/settingsdialog.cpp \
    src/file.cpp \
    src/filemodel.cpp \
    src/addeditdatasource.cpp \
    src/settings.cpp \
    src/extensions/productview/productview.cpp \
    src/filtersdialog.cpp \
    src/metadata.cpp \
    src/zimautils.cpp \
    src/errordialog.cpp \
    src/filefiltermodel.cpp \
    src/filefilters/filefilter.cpp \
    src/filefilters/extensionfilter.cpp \
    src/filefilters/filtergroup.cpp \
    src/filefilters/versionfilter.cpp \
    src/extensions/productview/abstractproductview.cpp \
    src/extensions/productview/proeproductview.cpp \
    src/extensions/productview/dxfproductview.cpp \
    libqdxf/libdxfrw/src/drw_base.h \
    libqdxf/libdxfrw/src/drw_entities.cpp \
    libqdxf/libdxfrw/src/drw_entities.h \
    libqdxf/libdxfrw/src/drw_interface.h \
    libqdxf/libdxfrw/src/drw_objects.cpp \
    libqdxf/libdxfrw/src/drw_objects.h \
    libqdxf/libdxfrw/src/libdxfrw.cpp \
    libqdxf/libdxfrw/src/libdxfrw.h \
    libqdxf/libdxfrw/src/intern/drw_cptable932.h \
    libqdxf/libdxfrw/src/intern/drw_cptable936.h \
    libqdxf/libdxfrw/src/intern/drw_cptable949.h \
    libqdxf/libdxfrw/src/intern/drw_cptable950.h \
    libqdxf/libdxfrw/src/intern/drw_cptables.h \
    libqdxf/libdxfrw/src/intern/drw_textcodec.cpp \
    libqdxf/libdxfrw/src/intern/drw_textcodec.h \
    libqdxf/libdxfrw/src/intern/dxfreader.cpp \
    libqdxf/libdxfrw/src/intern/dxfreader.h \
    libqdxf/libdxfrw/src/intern/dxfwriter.cpp \
    libqdxf/libdxfrw/src/intern/dxfwriter.h \
    libqdxf/src/dxfinterface.cpp \
    libqdxf/src/dxfsceneview.cpp \
    libqdxf/src/mtexttohtml.cpp \
    libqdxf/src/spline.cpp \
    libproe/libproe.cpp \
    src/extensions/productview/pdfproductview.cpp \
    src/extensions/productview/failbackproductview.cpp \
    src/languageflagswidget.cpp \
    src/extensions/navbar/navbar.cpp \
    src/extensions/navbar/navbarheader.cpp \
    src/extensions/navbar/navbaroptionsdialog.cpp \
    src/extensions/navbar/navbarpagelistwidget.cpp \
    src/extensions/navbar/navbarsplitter.cpp \
    src/workingdirwidget.cpp \
    src/fileview.cpp \
    src/webdownloaderwidget.cpp \
    src/webdownloaderdialog.cpp \
    src/progressdialog.cpp \
    src/directoryremover.cpp \
    src/extensions/productview/imageproductview.cpp \
    src/thumbnailmanager.cpp \
    src/webauthenticationdialog.cpp \
    src/filedelegate.cpp \
    src/fileeditdialog.cpp \
    src/fileviewheader.cpp \
    src/createdirectorydialog.cpp \
    src/directorycreator.cpp \
    src/threadworker.cpp \
    src/directoryeditordialog.cpp \
    src/directorylocaleeditwidget.cpp \
    src/directoryeditparametersmodel.cpp \
    src/metadata/metadatamigration.cpp \
    src/metadata/metadatamigrator.cpp \
    src/metadata/migrations/metadatav2migration.cpp \
    src/filecopier.cpp \
    src/datasourcewidget.cpp \
    src/directorywidget.cpp \
    src/datasourceview.cpp \
    src/datasourcemodel.cpp \
    src/directorywebview.cpp \
    src/maintabwidget.cpp \
    src/maintoolbar.cpp \
    src/datasourcehistory.cpp \
    src/partselector.cpp

HEADERS += src/mainwindow.h \
    src/settingsdialog.h \
    src/file.h \
    src/filemodel.h \
    src/addeditdatasource.h \
    src/settings.h \
    src/extensions/productview/productview.h \
    src/filtersdialog.h \
    src/metadata.h \
    src/zima-cad-parts.h \
    src/zimautils.h \
    src/errordialog.h \
    src/filefiltermodel.h \
    src/filefilters/filefilter.h \
    src/filefilters/extensionfilter.h \
    src/filefilters/filtergroup.h \
    src/filefilters/versionfilter.h \
    src/extensions/productview/abstractproductview.h \
    src/extensions/productview/proeproductview.h \
    src/extensions/productview/dxfproductview.h \
    libqdxf/src/mtexttohtml.h \
    libqdxf/src/scene_items.h \
    libqdxf/src/dxfsceneview.h \
    libqdxf/src/dxfinterface.h \
    libqdxf/src/spline.h \
    libproe/libproe.h \
    src/extensions/productview/pdfproductview.h \
    src/extensions/productview/failbackproductview.h \
    src/languageflagswidget.h \
    src/extensions/navbar/navbar.h \
    src/extensions/navbar/navbarheader.h \
    src/extensions/navbar/navbaroptionsdialog.h \
    src/extensions/navbar/navbarpage.h \
    src/extensions/navbar/navbarpagelistwidget.h \
    src/extensions/navbar/navbarsplitter.h \
    src/workingdirwidget.h \
    src/fileview.h \
    src/webdownloaderwidget.h \
    src/webdownloaderdialog.h \
    src/progressdialog.h \
    src/directoryremover.h \
    src/extensions/productview/imageproductview.h \
    src/thumbnailmanager.h \
    src/webauthenticationdialog.h \
    src/filedelegate.h \
    src/fileeditdialog.h \
    src/fileviewheader.h \
    src/createdirectorydialog.h \
    src/directorycreator.h \
    src/threadworker.h \
    src/directoryeditordialog.h \
    src/directorylocaleeditwidget.h \
    src/directoryeditparametersmodel.h \
    src/metadata/metadatamigration.h \
    src/metadata/metadatamigrator.h \
    src/metadata/migrations/metadatav2migration.h \
    src/filecopier.h \
    src/datasourcewidget.h \
    src/directorywidget.h \
    src/datasourceview.h \
    src/datasourcemodel.h \
    src/directorywebview.h \
    src/maintabwidget.h \
    src/maintoolbar.h \
    src/datasourcehistory.h \
    src/partselector.h

FORMS += mainwindow.ui \
    settingsdialog.ui \
    addeditdatasource.ui \
    src/filtersdialog.ui \
    src/errordialog.ui \
    src/extensions/productview/proeproductview.ui \
    src/extensions/productview/dxfproductview.ui \
    src/extensions/productview/pdfproductview.ui \
    src/extensions/productview/failbackproductview.ui \
    src/extensions/navbar/navbaroptionsdialog.ui \
    src/workingdirwidget.ui \
    src/webdownloaderwidget.ui \
    src/webdownloaderdialog.ui \
    src/progressdialog.ui \
    src/extensions/productview/imageproductview.ui \
    src/webauthenticationdialog.ui \
    src/fileeditdialog.ui \
    src/createdirectorydialog.ui \
    src/directoryeditordialog.ui \
    src/directorylocaleeditwidget.ui \
    src/datasourcewidget.ui \
    src/directorywidget.ui \
    src/maintabwidget.ui \
    src/maintoolbar.ui

RESOURCES += zima-cad-parts.qrc \
    src/extensions/navbar/navbar.qrc

OTHER_FILES += \
    src/zima-cad-parts.rc \
    LICENSE \
    AUTHORS \
    data/zima-cad-parts.html \
    data/zima-cad-parts_cs_CZ.html \
    data/extensions/productview/proeproductview.html \
    Doxyfile \
    src/extensions/navbar/styles/downarrowblue.png \
    src/extensions/navbar/styles/sizegrip2003blue.png \
    src/extensions/navbar/styles/sizegrip2003gray.png \
    src/extensions/navbar/styles/sizegrip2003green.png \
    src/extensions/navbar/styles/sizegrip2003silver.png \
    src/extensions/navbar/styles/sizegrip2007black.png \
    src/extensions/navbar/styles/sizegrip2007blue.png \
    src/extensions/navbar/styles/sizegrip2007silver.png \
    src/extensions/navbar/styles/splitter2003blue.png \
    src/extensions/navbar/styles/splitter2003gray.png \
    src/extensions/navbar/styles/splitter2003green.png \
    src/extensions/navbar/styles/splitter2003silver.png \
    src/extensions/navbar/styles/splitter2007black.png \
    src/extensions/navbar/styles/splitter2007blue.png \
    src/extensions/navbar/styles/splitter2007silver.png \
    src/extensions/navbar/styles/office2003blue.css \
    src/extensions/navbar/styles/office2003gray.css \
    src/extensions/navbar/styles/office2003green.css \
    src/extensions/navbar/styles/office2003silver.css \
    src/extensions/navbar/styles/office2007black.css \
    src/extensions/navbar/styles/office2007blue.css \
    src/extensions/navbar/styles/office2007silver.css \
    src/extensions/navbar/COPYING

TRANSLATIONS = locale/zima-cad-parts_cs_CZ.ts

win32:CONFIG += static
greaterThan(QT_MAJOR_VERSION, 4) {
    win32:RC_ICONS = gfx/icon.ico
}
else {
    win32:RC_FILE = src/zima-cad-parts.rc
}

ICON = gfx/icon.icns

DISTFILES += \
    README.md \
    doc/datasource.md \
    doc/metadata.md \
    doc/users.md
