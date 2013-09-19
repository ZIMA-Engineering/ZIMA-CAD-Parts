# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network webkit
TARGET = ZIMA-CAD-Parts
TEMPLATE = app
win32:INCLUDEPATH += ../
VPATH += ./src
SOURCES += zima-cad-parts.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    serversmodel.cpp \
    item.cpp \
    filemodel.cpp \
    basedatasource.cpp \
    baseremotedatasource.cpp \
    ftpdatasource.cpp \
    localdatasource.cpp \
    addeditdatasource.cpp \
    downloadmodel.cpp \
    src/extensions/productview/productviewsettings.cpp \
    src/extensions/productview/productview.cpp \
    src/filtersdialog.cpp \
    src/metadata.cpp \
    src/techspecswebview.cpp \
    src/zimautils.cpp \
    src/errordialog.cpp \
    src/treeautodescent.cpp \
    src/thumbnail.cpp \
    src/filefiltermodel.cpp \
    src/filefilters/filefilter.cpp \
    src/filefilters/extensionfilter.cpp \
    src/filefilters/filtergroup.cpp \
    src/filefilters/versionfilter.cpp
HEADERS += mainwindow.h \
    settingsdialog.h \
    serversmodel.h \
    item.h \
    filemodel.h \
    basedatasource.h \
    baseremotedatasource.h \
    ftpdatasource.h \
    localdatasource.h \
    addeditdatasource.h \
    downloadmodel.h \
    src/extensions/productview/productviewsettings.h \
    src/extensions/productview/productview.h \
    src/filtersdialog.h \
    src/metadata.h \
    src/techspecswebview.h \
    src/zima-cad-parts.h \
    src/zimautils.h \
    src/errordialog.h \
    src/treeautodescent.h \
    src/thumbnail.h \
    src/filefiltermodel.h \
    src/filefilters/filefilter.h \
    src/filefilters/extensionfilter.h \
    src/filefilters/filtergroup.h \
    src/filefilters/versionfilter.h
FORMS += mainwindow.ui \
    settingsdialog.ui \
    addeditdatasource.ui \
    src/extensions/productview/productviewsettings.ui \
    src/extensions/productview/productview.ui \
    src/filtersdialog.ui \
    src/errordialog.ui
RESOURCES += zima-cad-parts.qrc

OTHER_FILES += \
    zima-cad-parts.rc \
    LICENSE \
    AUTHORS \
    data/extensions/productview/productview.html \
    data/zima-cad-parts.html \
    data/zima-cad-parts_cs_CZ.html \
    README

TRANSLATIONS = locale/zima-cad-parts_cs_CZ.ts

win32:CONFIG += static
win32:RC_FILE = src/zima-cad-parts.rc

ICON = gfx/icon.icns
