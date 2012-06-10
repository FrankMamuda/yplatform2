/*
===========================================================================
Copyright (C) 2009-2012 Edd 'Double Dee' Psycho

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

//
// includes
//
#include "../applet/app_main.h"
#include "sys_common.h"
#include <sys/time.h>

//
// classes
//
class Sys_Common com;

/*
============
print
============
*/
void Sys_Common::print( const QString &msg, int fontSize ) {
    // print out
    fputs( msg.toLatin1().constData(), stdout );
    if ( this->gui()->hasInitialized())
        this->gui()->print( msg, fontSize );

    // for debugging
    qDebug() << msg.left( msg.length()-1 );
}

/*
============
error
============
*/
void Sys_Common::error( ErrorTypes type, const QString &msg ) {
    if ( this->gui()->hasInitialized()) {
        com.gui()->printImage( "icons/panic", 16, 16 );
        com.gui()->print( " " );
    }

    if ( type == FatalError )
        this->print( Sys::cRed + this->tr( "FATAL ERROR: %1" ).arg( msg ));
    else
        this->print( Sys::cRed + this->tr( "ERROR: %1" ).arg( msg ));

    if ( type == FatalError ) {
        this->catchError();
        com.gui()->freeze();
    }
}

/*
================
milliseconds
================
*/
int Sys_Common::milliseconds() {
    return m.time.elapsed();
}
