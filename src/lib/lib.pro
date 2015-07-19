#-------------------------------------------------
#
# Project created by QtCreator 2015-04-25T00:31:28
#
#-------------------------------------------------

QT       += network
QT       -= gui
QMAKE_CXXFLAGS += -std=c++11

include(../../qmikrotik.pri)
DESTDIR = $$ROOT_DIR/lib

TARGET = qmikrotik
TEMPLATE = lib

DEFINES += SRC_LIBRARY

SOURCES += \
    mrouter.cpp \
    mcommand.cpp

HEADERS += \
    mikrotik_global.h \
    mrouter.h \
    mrouter_p.h \
    endianhelper.h \
    mcommand.h \
    mcommand_p.h
