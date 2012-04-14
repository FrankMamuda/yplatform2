/*
===========================================================================
Copyright (C) 2011-2012 Edd 'Double Dee' Psycho

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef UI_MAIN_H
#define UI_MAIN_H

//
// includes
//
#include "module_global.h"
#include "../mod_trap.h"
#include "ui_script.h"
#include <QtGui>

//
// classes
//
class UiItem;
class UI_ScriptEngine;

//
// namespaces
//
namespace UI {
    static const QString Title        ( QObject::trUtf8( "User Interface" ));
    static const QString Copyright    ( QObject::trUtf8( "Copyright (c) 2011-2012, Edd 'Double Dee' Psycho." ));
    static const QString Version      ( QObject::trUtf8( "v0.2.1 alpha" ));
    typedef float Vec4D[4];
}

//
// class::UI_Main
//
class UiMain : public QObject {
    Q_OBJECT
    Q_ENUMS( Context )

public:
    enum Context {
        Default = 0,
        Foreground = 1,
        Background = 2
    };
    UiMain() {}
    QList<UiItem*>itemList;
    QPoint mousePos;
    UI_ScriptEngine *uiS;
    Context currentContext() const { return this->m_currentContext; }

public slots:
    void setForeground() { this->m_currentContext = Foreground; }
    void setBackground() { this->m_currentContext = Background; }
    void setDefaultContext() { this->m_currentContext = Default; }
    void update();
    void init();
    void render();
    void shutdown();

private:
    Context m_currentContext;
};

//
// externals
//
extern class UiMain m;

//
// metatypes
//
Q_DECLARE_METATYPE( UiMain::Context )

#endif // UI_MAIN_H
