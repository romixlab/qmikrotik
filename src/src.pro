#-------------------------------------------------
#
# Project created by QtCreator 2015-04-25T00:31:28
#
#-------------------------------------------------

QT       += network

QT       -= gui

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
