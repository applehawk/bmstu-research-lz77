# -------------------------------------------------
# Project created by QtCreator 2009-11-14T23:19:32
# -------------------------------------------------
TEMPLATE = app
DESTDIR = ../bin

debug {
  TARGET = gui_d
}
release {
  TARGET = gui
}
# путь до LZX библиотек метода сжатия
LIBS += $$quote(-L../lzx/lib)
debug:LIBS += liblzxd
!debug:LIBS += liblzx
SOURCES += main.cpp \
    archproj.cpp \
    statusdlg.cpp \
    langchooser.cpp \
    filelistwidget.cpp \
    extensionmng.cpp \
    decompressdlg.cpp \
    compressdlg.cpp \
    paf.cpp \
    ../methods/zlib_method.cpp \
    ../methods/suffix_lz_method.cpp \
    ../methods/lzx_method.cpp \
    ../methods/archive_method.cpp \
    archiveinfodlg.cpp
HEADERS += archproj.h \
    statusdlg.h \
    langchooser.h \
    filelistwidget.h \
    ../lzx/include/types_lzx.h \
    extensionmng.h \
    decompressdlg.h \
    compressdlg.h \
    paf.h \
    ../methods/zlib_method.h \
    ../methods/suffix_lz_method.h \
    ../methods/lzx_method.h \
    ../methods/archive_method.h \
    archiveinfodlg.h \
    ../lzx/include/types_lzx.h \
    ../lzx/include/lzx.h
