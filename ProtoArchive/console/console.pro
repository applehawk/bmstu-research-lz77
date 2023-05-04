# -------------------------------------------------
# Project created by QtCreator 2009-11-18T22:51:10
# -------------------------------------------------
QT -= gui
DESTDIR = ../bin
TARGET = console

# путь до LZX библиотек метода сжатия
LIBS += $$quote(-L../lzx/lib)
debug:LIBS += liblzx_d
release:LIBS += liblzx
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
DEFINES += DEBUG_TIMESTAMP_ENABLE
SOURCES += main.cpp \
    dtimestamp.cpp \
    ../methods/zlib_method.cpp \
    ../methods/suffix_lz_method.cpp \
    ../methods/lzx_method.cpp \
    ../methods/archive_method.cpp
HEADERS += dtimestamp.h \
    ../methods/zlib_method.h \
    ../methods/suffix_lz_method.h \
    ../methods/status_updater_lzx.h \
    ../methods/lzx_method.h \
    ../methods/archive_method.h
