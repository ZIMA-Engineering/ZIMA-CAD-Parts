QT = core
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = ZIMA-CAD-Parts-cli
SOURCES += src/cli/main.cpp src/core/partsread.cpp src/core/partsquery.cpp \
    src/metadata/metadatamigration.cpp src/metadata/migrations/metadatav2migration.cpp
HEADERS += src/core/partsread.h src/core/partsquery.h src/localfilters.h
!versionAtLeast(QT_VERSION, 6.8.0): error(Qt 6.8 or newer is required.)

SOURCES += src/core/partscommand.cpp
HEADERS += src/core/partscommand.h

SOURCES += src/core/partstools.cpp
HEADERS += src/core/partstools.h

SOURCES += src/core/partsupdatecommand.cpp
HEADERS += src/core/partsupdatecommand.h
SOURCES += src/update/installationclient.cpp
HEADERS += src/update/installationclient.h

# Windows PTC Cleaner uses one native Shell recycle operation per selection.
win32:LIBS += -lole32 -lshell32 -luuid
