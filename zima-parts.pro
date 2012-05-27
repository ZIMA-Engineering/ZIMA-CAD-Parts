# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network webkit
TARGET = zima-parts
TEMPLATE = app
win32:INCLUDEPATH += ../
VPATH += ./src
SOURCES += zima-parts.cpp \
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
    src/filtersdialog.cpp
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
    src/zima-parts.h \
    src/extensions/productview/productviewsettings.h \
    src/extensions/productview/productview.h \
    src/filtersdialog.h
FORMS += mainwindow.ui \
    settingsdialog.ui \
    addeditdatasource.ui \
    src/extensions/productview/productviewsettings.ui \
    src/extensions/productview/productview.ui \
    src/filtersdialog.ui
RESOURCES += zima-parts.qrc

OTHER_FILES += data/zima-parts.html \
    zima-parts.rc \
    LICENSE \
    AUTHORS \
    data/zima-parts_cs_CZ.html \
    data/extensions/productview/productview.html

TRANSLATIONS = locale/zima-parts_cs_CZ.ts

win32:CONFIG += static
win32:RC_FILE = src/zima-parts.rc






