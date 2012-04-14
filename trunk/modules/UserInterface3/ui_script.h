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

#ifndef UI_SCRIPT_H
#define UI_SCRIPT_H

//
// includes
//
#include "ui_main.h"
#include "../mod_script.h"

//
// class:UI_ScriptEngine
//
class UI_ScriptEngine : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "UiScript engine" )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )

public:
    explicit UI_ScriptEngine();
    ~UI_ScriptEngine();

    // property getters
    bool hasInitialized() const { return this->m_initialized; }

    // scripting engine
    Mod_ScriptEngine *mse;

private:
    // properties
    bool m_initialized;

signals:

public slots:
    void loadUIScript( const QStringList &args );
    void evaluateUIScript( const QStringList &args );

    // property setters
    void setInitialized( bool initialized = true ) { this->m_initialized = initialized; }
};

#endif // UI_SCRIPT_H
