# -------------------------------------------------
# Project created by QtCreator 2010-03-29T21:22:07
# -------------------------------------------------
QT += opengl
QT += script
TARGET = Renderer
TEMPLATE = lib
DEFINES += R_BUILD
DEFINES += MODULE_LIBRARY
win32:INCLUDEPATH += ../includes
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
    r_settings.cpp
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
    r_settings.h
OTHER_FILES += CHANGELOG.txt \
    assets/smth.qml \
    resources/materials/common.js
RESOURCES += \
    resources/r_assets.qrc

FORMS += \
    r_settings.ui
