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

HEADERS += \
    applet/app_main.h \
    common/sys_shared.h \
    common/sys_common.h \
    common/sys_cmd.h \
    common/sys_cvar.h \
    common/sys_filesystem.h \
    common/sys_module.h \
    common/sys_cvarfunc.h \
    common/sys_modulefunc.h \
    gui/gui_main.h \
    gui/gui_about.h \
    modules/mod_public.h \
    common/sys_package.h \
    common/sys_packagefunc.h \
    gui/gui_settings.h \
    gui/gui_license.h

win32:HEADERS += common/sys_link.h
win32:RC_FILE = resources/platform/YPlatform2.rc

SOURCES += \
    applet/app_main.cpp \
    common/sys_common.cpp \
    common/sys_cmd.cpp \
    common/sys_cvar.cpp \
    common/sys_filesystem.cpp \
    common/sys_module.cpp \
    common/sys_cvarfunc.cpp \
    common/sys_modulefunc.cpp \
    gui/gui_main.cpp \
    gui/gui_about.cpp \
    common/sys_package.cpp \
    common/sys_packagefunc.cpp \
    gui/gui_settings.cpp \
    gui/gui_license.cpp

win32:INCLUDEPATH += includes

QT += xml
QT += script

FORMS += \
    gui/gui_main.ui \
    gui/gui_about.ui \
    gui/gui_settings.ui \
    gui/gui_license.ui

OTHER_FILES += \
    CHANGELOG.txt

RESOURCES += \
    resources/res_icons.qrc

TRANSLATIONS = i18n/YPlatform2_lv_LV.ts
