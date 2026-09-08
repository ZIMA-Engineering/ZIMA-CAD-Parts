QT += core gui widgets testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = parts-performance-tests
INCLUDEPATH += ../src
SOURCES += tst_partsperformance.cpp ../src/thumbnailmanager.cpp ../src/file.cpp
HEADERS += ../src/thumbnailmanager.h ../src/localfilters.h ../src/file.h
