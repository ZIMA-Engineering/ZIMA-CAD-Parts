TEMPLATE = app
CONFIG += console
mac {
  CONFIG -= app_bundle
}
TARGET = proe-cli
INCLUDEPATH += .

# Input
HEADERS += libproe.h
SOURCES += libproe.cpp proe-cli.cpp
