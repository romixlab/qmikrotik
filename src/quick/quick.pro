TEMPLATE = lib
TARGET = quick
QT += qml quick
CONFIG += qt plugin

include(../../qmikrotik.pri)
RCC_DIR = $$BUILD_DIR/$$TARGET/rcc_files
UI_DIR = $$BUILD_DIR/$$TARGET/uic_files
MOC_DIR = $$BUILD_DIR/$$TARGET/moc_files
OBJECTS_DIR = $$BUILD_DIR/$$TARGET/obj_files

DESTDIR = $$ROOT_DIR/qml

INCLUDEPATH += $$ROOT_DIR/src/lib
LIBS += -L"$$ROOT_DIR/lib" -lqmikrotik

TARGET = $$qtLibraryTarget($$TARGET)
uri = com.romixlab.qmlcomponents

# Input
SOURCES += \
    quick_plugin.cpp

HEADERS += \
    quick_plugin.h

DISTFILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

