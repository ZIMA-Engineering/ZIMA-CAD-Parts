QT += network opengl openglwidgets widgets webchannel webenginewidgets testlib
# Match the optional CAD and platform credential libraries of the app objects.
include(../src/extensions/productview/occt.pri)
include(../3rdparty/qtkeychain/qtkeychain.pri)
win32:LIBS += -lcrypt32
SOURCES =
HEADERS =
FORMS =
OBJECTIVE_SOURCES =
DBUS_INTERFACES =

CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = parts-integration-tests
INCLUDEPATH += ../src
SOURCES += tst_partsintegration.cpp

# Reuse a matching release build of the application; exclude its main().
isEmpty(PARTS_OBJECTS_DIR): error("Pass PARTS_OBJECTS_DIR pointing to the application's release objects")
APP_OBJECTS = $$files($$PARTS_OBJECTS_DIR/*.obj) $$files($$PARTS_OBJECTS_DIR/*.o)
APP_OBJECTS -= $$PARTS_OBJECTS_DIR/zima-cad-parts.obj $$PARTS_OBJECTS_DIR/zima-cad-parts.o
LIBS += $$APP_OBJECTS
PRE_TARGETDEPS += $$APP_OBJECTS
INCLUDEPATH += $$PARTS_OBJECTS_DIR/..
INCLUDEPATH += ..
