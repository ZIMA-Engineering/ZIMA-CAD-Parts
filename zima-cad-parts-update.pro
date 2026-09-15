QT = core network
QT += core-private
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = ZIMA-CAD-Parts-update
SOURCES += src/update/updatemain.cpp src/update/updatecore.cpp src/update/updatehttp.cpp src/update/updateinstall.cpp
HEADERS += src/update/updatecore.h
RESOURCES += src/update/update.qrc
win32 {
    isEmpty(OPENSSL_ROOT): error(Pass OPENSSL_ROOT for the OpenSSL 3 installation)
    INCLUDEPATH += $$OPENSSL_ROOT/include
    LIBS += -L$$OPENSSL_ROOT/lib -llibcrypto
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += openssl
}
contains(CONFIG, update_tests): DEFINES += PARTS_UPDATE_TESTING
