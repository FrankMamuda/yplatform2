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

#ifndef R_MTRLIB_H
#define R_MTRLIB_H

//
// includes
//
#include "r_shared.h"
#include "r_material.h"
#include "../modules/mod_script.h"

//
// class:R_MtrLib
//
class R_MtrLib : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "MtrLib engine" )

private:
    // properties
    bool m_initialized;

public:
    // property getters
    bool hasInitialized() const { return this->m_initialized; }
    R_Image::ClampModes getClampMode( const QString & );

    // scripting engine related
    Mod_ScriptEngine *mse;

signals:

public slots:
    void init();
    void shutdown();

    // property setters
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
};

//
// externals
//
#ifdef R_BUILD
extern class R_MtrLib mLib;
#endif

#endif // R_MTRLIB_H
