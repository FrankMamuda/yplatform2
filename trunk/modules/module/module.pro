#-------------------------------------------------
#
# Project created by QtCreator 2011-01-03T14:57:57
#
#-------------------------------------------------

QT       -= gui

TARGET = module
TEMPLATE = lib

DEFINES += MODULE_LIBRARY

SOURCES += mod_main.cpp \
    ../mod_trap.cpp \
    ../mod_cvarfunc.cpp
DEFINES += MODULE_BUILD
HEADERS += mod_main.h\
        module_global.h \
    ../mod_public.h \
    ../mod_trap.h
