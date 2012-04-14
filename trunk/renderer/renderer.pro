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

QT += opengl
QT += script
TARGET = Renderer
TEMPLATE = lib
DEFINES += R_BUILD
DEFINES += MODULE_BUILD
win32:INCLUDEPATH += ../includes
INCLUDEPATH += ../common
INCLUDEPATH += ../modules
SOURCES += r_public.cpp \
    r_glimp.cpp \
    ../modules/mod_trap.cpp \
    ../modules/mod_cvarfunc.cpp \
    r_glimpwidget.cpp \
    r_image.cpp \
    r_main.cpp \
    r_cmd.cpp \
    r_material.cpp \
    r_mtrlib.cpp \
    r_materialstage.cpp \
    r_settings.cpp \
    ../modules/mod_script.cpp
HEADERS += r_public.h \
    renderer_global.h \
    r_glimp.h \
    ../modules/mod_trap.h \
    ../modules/mod_public.h \
    ../modules/mod_cvarfunc.h \
    r_glimpwidget.h \
    r_image.h \
    r_main.h \
    r_cmd.h \
    r_shared.h \
    r_material.h \
    r_mtrlib.h \
    r_materialstage.h \
    r_settings.h \
    ../common/sys_filesystem.h \
    ../common/sys_cvarfunc.h \
    ../common/sys_common.h \
    ../modules/mod_script.h
OTHER_FILES += CHANGELOG.txt \
    assets/smth.qml \
    resources/materials/common.js
RESOURCES += \
    resources/r_assets.qrc

FORMS += \
    r_settings.ui
