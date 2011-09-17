# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network webkit
TARGET = zimaparts
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    ftpdata.cpp \
    serversmodel.cpp \
    item.cpp \
    ftpserver.cpp \
    filemodel.cpp
HEADERS += mainwindow.h \
    settingsdialog.h \
    ftpdata.h \
    ftpserver.h \
    serversmodel.h \
    item.h \
    filemodel.h
FORMS += mainwindow.ui \
    settingsdialog.ui
RESOURCES += zimaparts.qrc

OTHER_FILES += \
    zimaparts.html
CONFIG += static
