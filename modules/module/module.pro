#
# Copyright (C) 2009-2012 Edd 'Double Dee' Psycho
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.
#

QT += script

win32:INCLUDEPATH += ../../includes

TARGET = module
TEMPLATE = lib

DEFINES += MODULE_BUILD

INCLUDEPATH += ../../common
INCLUDEPATH += ../

SOURCES += mod_main.cpp \
    ../mod_trap.cpp \
    ../mod_cvarfunc.cpp
DEFINES += MODULE_BUILD
HEADERS += mod_main.h\
        module_global.h \
    ../mod_public.h \
    ../mod_trap.h \
    ../mod_cvarfunc.h

