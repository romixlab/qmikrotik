#-------------------------------------------------
#
# Project created by QtCreator 2015-04-25T00:31:28
#
#-------------------------------------------------

QT       += network

QT       -= gui

include(../../qmikrotik.pri)
RCC_DIR = $$BUILD_DIR/$$TARGET/rcc_files
UI_DIR = $$BUILD_DIR/$$TARGET/uic_files
MOC_DIR = $$BUILD_DIR/$$TARGET/moc_files
OBJECTS_DIR = $$BUILD_DIR/$$TARGET/obj_files

CONFIG(debug, debug|release) {
    DESTDIR = $$ROOT_DIR/debug/lib
}
CONFIG(release, debug|release) {
    DESTDIR = $$ROOT_DIR/lib
}


TARGET = qmikrotik
TEMPLATE = lib

DEFINES += SRC_LIBRARY

SOURCES += \
    router.cpp

HEADERS += \
    mikrotik_global.h \
    router.h \
    router_p.h \
    sysdep.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
