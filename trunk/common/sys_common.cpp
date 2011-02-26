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

//
// includes
//
#include "../applet/app_main.h"
#include "sys_common.h"
#include <sys/time.h>

//
// classes
//
extern class App_Main m;
class Sys_Common com;

/*
============
print
============
*/
void Sys_Common::print( const QString &msg, int fontSize ) {
    // print out
    fputs( msg.toLatin1().constData(), stdout );
    if ( this->gui->initialized )
        this->gui->print( msg, fontSize );

    // for debugging
    qDebug() << msg.left( msg.length()-1 );
}

/*
============
error
============
*/
void Sys_Common::error( int type, const QString &msg ) {
    if ( this->gui->initialized ) {
        com.gui->printImage( ":/icons/panic", 16, 16 );
        com.gui->print( " " );
    }

    if ( type == ERR_FATAL )
        this->print( this->tr( "^1FATAL ERROR: %1" ).arg( msg ));
    else
        this->print( this->tr( "^3ERROR: %1" ).arg( msg ));

    if ( type == ERR_FATAL ) {
        this->caughtFatalError = true;
        com.gui->freeze();
    }
}

/*
================
milliseconds
================
*/
int Sys_Common::milliseconds() {
    int curtime;
    struct timeval tp;
    gettimeofday( &tp, NULL );

    if ( !timeBase ) {
        timeBase = tp.tv_sec;
        return tp.tv_usec / 1000;
    }

    curtime = ( tp.tv_sec - timeBase ) * 1000 + tp.tv_usec / 1000;
    return curtime;
}

/*
================
shortSwap
=================
*/
short shortSwap( short l ) {
    byte b1, b2;

    b1 = l&255;
    b2 = ( l >> 8 )&255;

    return ( b1 << 8 ) + b2;
}

/*
================
longSwap
=================
*/
int longSwap( int l ) {
    byte b1, b2, b3, b4;

    b1 = l&255;
    b2 = ( l >> 8 )&255;
    b3 = ( l >> 16 )&255;
    b4 = ( l >> 24 )&255;

    return (( int )b1 << 24 ) + (( int )b2 << 16 ) + (( int )b3 << 8 ) + b4;
}

/*
================
floatSwap
=================
*/
float floatSwap( const float *f ) {
    floatByteUnion out;

    out.f = *f;
    out.i = longSwap( out.i );

    return out.f;
}

/*
=============
strncpyz
=============
*/
void Sys_Common::strncpyz( char *dest, const char *src, int destsize ) {
    if ( !dest )
        this->print( this->tr( "Sys_Common::strncpyz: NULL dest\n" ));

    if ( !src )
        this->print( this->tr( "Sys_Common::strncpyz: NULL src\n" ));

    if ( destsize < 1 )
        this->print( this->tr( "Sys_Common::strncpyz: destsize < 1\n" ));

    strncpy( dest, src, destsize-1 );
    dest[destsize-1] = 0;
}

