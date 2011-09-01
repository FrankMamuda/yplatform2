/*
===========================================================================
Copyright (C) 2011 Edd 'Double Dee' Psycho

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
#include <QtScript>

//
// class:R_MtrLib
//
class R_MtrLib : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Material library parsing engine" )
    Q_PROPERTY( bool pError READ hasCaughtError WRITE catchError )

private:
    void loadMtrLib( const QString &filename );
    bool hasCaughtError() const { return this->m_pError; }

    // scripting engine relates
    QScriptValue object;
    QScriptValue material;
    QScriptEngine engine;
    QScriptValue updateFunc;

    // properties
    bool m_initialized;
    bool m_pError;

public:
    // property getters
    bool hasInitialized() const { return this->m_initialized; }
    R_Image::ClampModes getClampMode( const QString & );

signals:

public slots:
    void init();
    void update();
    void shutdown();

    // property setters
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void catchError( bool error = true ) { this->m_pError = error; }
};

//
// externals
//
#ifdef R_BUILD
extern class R_MtrLib mLib;
#endif

#endif // R_MTRLIB_H
