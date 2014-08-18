# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network webkit opengl
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets webkitwidgets
}
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
#greaterThan(QT_MAJOR_VERSION, 4) {
#    win32 {
#        INCLUDEPATH += win32/poppler-0.24.5-win32/include
#        LIBS += -L$$PWD/win32/poppler-0.24.5-win32/bin
#        LIBS += -lpoppler-qt5
#    }
#    unix {
#        CONFIG += link_pkgconfig
#        PKGCONFIG += poppler-qt5
#    }
#}
#else {
#    unix {
#        CONFIG += link_pkgconfig
#        PKGCONFIG += poppler-qt4
#    }
#}

SOURCES += src/zima-cad-parts.cpp \
    src/mainwindow.cpp \
    src/settingsdialog.cpp \
    src/serversmodel.cpp \
    src/file.cpp \
    src/filemodel.cpp \
    src/addeditdatasource.cpp \
    src/settings.cpp \
    src/extensions/productview/productview.cpp \
    src/filtersdialog.cpp \
    src/metadata.cpp \
    src/techspecswebview.cpp \
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
    src/extensions/productview/pdfproductview.cpp \
    src/extensions/productview/failbackproductview.cpp \
    src/serverswidget.cpp \
    src/servertabwidget.cpp \
    src/languageflagswidget.cpp \
    src/extensions/navbar/navbar.cpp \
    src/extensions/navbar/navbarheader.cpp \
    src/extensions/navbar/navbaroptionsdialog.cpp \
    src/extensions/navbar/navbarpagelistwidget.cpp \
    src/extensions/navbar/navbarsplitter.cpp \
    src/workingdirwidget.cpp \
    src/serversview.cpp \
    src/fileview.cpp

HEADERS += src/mainwindow.h \
    src/settingsdialog.h \
    src/serversmodel.h \
    src/file.h \
    src/filemodel.h \
    src/addeditdatasource.h \
    src/settings.h \
    src/extensions/productview/productview.h \
    src/filtersdialog.h \
    src/metadata.h \
    src/techspecswebview.h \
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
    src/extensions/productview/pdfproductview.h \
    src/extensions/productview/failbackproductview.h \
    src/serverswidget.h \
    src/servertabwidget.h \
    src/languageflagswidget.h \
    src/extensions/navbar/navbar.h \
    src/extensions/navbar/navbarheader.h \
    src/extensions/navbar/navbaroptionsdialog.h \
    src/extensions/navbar/navbarpage.h \
    src/extensions/navbar/navbarpagelistwidget.h \
    src/extensions/navbar/navbarsplitter.h \
    src/workingdirwidget.h \
    src/serversview.h \
    src/fileview.h

FORMS += mainwindow.ui \
    settingsdialog.ui \
    addeditdatasource.ui \
    src/filtersdialog.ui \
    src/errordialog.ui \
    src/extensions/productview/proeproductview.ui \
    src/extensions/productview/productview.ui \
    src/extensions/productview/dxfproductview.ui \
    src/extensions/productview/pdfproductview.ui \
    src/extensions/productview/failbackproductview.ui \
    src/serverswidget.ui \
    src/servertabwidget.ui \
    src/extensions/navbar/navbaroptionsdialog.ui \
    src/workingdirwidget.ui

RESOURCES += zima-cad-parts.qrc \
    src/extensions/navbar/navbar.qrc

OTHER_FILES += \
    zima-cad-parts.rc \
    LICENSE \
    AUTHORS \
    data/zima-cad-parts.html \
    data/zima-cad-parts_cs_CZ.html \
    README \
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
    #win32:RC_ICONS += gfx/icon.ico
}
else {
    win32:RC_FILE = src/zima-cad-parts.rc
}

ICON = gfx/icon.icns
