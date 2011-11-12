/*
===========================================================================
Copyright (C) 2009-2011 Edd 'Double Dee' Psycho

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

#ifndef SYS_COMMON_H
#define SYS_COMMON_H

//
// includes
//
#include "sys_shared.h"
#ifndef MODULE_BUILD
#include "../gui/gui_main.h"
#endif

//
// namespaces
//
namespace Common {
    class Sys_Common;
}

//
// class:Sys_Common
//
class Sys_Common : public QObject {
    Q_OBJECT
#ifndef MODULE_BUILD
    Q_CLASSINFO( "description", "Platform common functions" )
    Q_PROPERTY( bool pError READ hasCaughtError WRITE catchError )
    Q_PROPERTY( Gui_Main *gui READ gui WRITE setGui )
#endif
    Q_ENUMS( ErrorTypes )

public:
    // enums
    enum ErrorTypes {
        SoftError = 0,
        FatalError
    };

    // byte order
    typedef union {
        float f;
        unsigned int i;
    } floatByteUnion;

    short shortSwap( short );
    int longSwap( int );
    float floatSwap( const float* );

#ifndef MODULE_BUILD
    void print( const QString &msg, int fontSize = 10 );
    void error( ErrorTypes type, const QString &msg );
    bool hasCaughtError() const { return this->m_pError; }
    Gui_Main *gui() { return this->m_gui; }
    int milliseconds();

private:
    unsigned long timeBase;
    bool m_pError;
    Gui_Main *m_gui;

signals:

public slots:
    void setGui( Gui_Main *gui ) { this->m_gui = gui; }
    void catchError( bool error = true ) { this->m_pError = error; }
#endif
};

//
// externals
//
extern class Sys_Common com;

#endif // SYS_COMMON_H
