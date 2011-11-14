# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network webkit
TARGET = zimaparts
TEMPLATE = app
win32:INCLUDEPATH += ../
VPATH += ./src
SOURCES += main.cpp \
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
    downloadmodel.cpp
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
    downloadmodel.h
FORMS += mainwindow.ui \
    settingsdialog.ui \
    addeditdatasource.ui
RESOURCES += zimaparts.qrc

OTHER_FILES += \
    data/zimaparts.html \
    zimaparts.rc \
    LICENSE
CONFIG += static
win32:RC_FILE = src/zimaparts.rc
