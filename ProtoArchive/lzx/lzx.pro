# -------------------------------------------------
# Project created by QtCreator 2009-11-15T13:33:56
# -------------------------------------------------
QT -= core \
    gui
DESTDIR = lib

 CONFIG(debug, debug|release) {
     mac: TARGET = $$join(TARGET,,,_debug)
     win32: TARGET = $$join(TARGET,,,d)
 }

INCLUDEPATH = include
TEMPLATE = lib
CONFIG += staticlib
SOURCES += src/lzx.cpp
HEADERS += include/types_lzx.h \
    include/lzx.h
