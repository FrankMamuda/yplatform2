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
#include "sys_cvarfunc.h"
#include "sys_common.h"
#include "sys_module.h"

//
// classes
//
extern class Sys_Common com;
extern class Sys_Module mod;

/*
============
construct
============
*/
pCvar::pCvar( const QString &name, const QString &string, int flags, const QString &desc, bool mCvar ) {
    // set the defaults
    this->flags = flags;
    this->name = name;
    this->stringValue = string;
    this->reset = string;
    this->latch = QString::null;
    this->description = desc;

    if ( mCvar )
        this->connect( this, SIGNAL( valueChanged( QString, QString )), &mod, SLOT( updateCvar( QString, QString )));
}

/*
============
destruct
============
*/
pCvar::~pCvar() {
}

/*
============
string
============
*/
QString pCvar::string() {
    return this->stringValue;
}

/*
============
integer
============
*/
int pCvar::integer() {
    bool valid;
    int y;

    // set integer if any
    y = this->stringValue.toInt( &valid );

    // all ok, return integer value
    if ( valid )
        return y;

    return 0;
}

/*
============
value
============
*/
float pCvar::value() {
    bool valid;
    float y;

    // set integer if any
    y = this->stringValue.toFloat( &valid );

    // all ok, return float value
    if ( valid )
        return y;

    return 0;
}

/*
============
set
============
*/
bool pCvar::set( const QString &string, bool force ) {
    if ( this->flags & CVAR_ROM && !force ) {
        com.print( this->tr( " ^1'%1' is read only\n" ).arg( this->name ));
    } else if ( this->flags & CVAR_PASSWORD/* && !com.developerMode */) {
        com.print( this->tr( " ^1'%1' is password protected\n" ).arg( this->name ));
    } else if ( this->flags & CVAR_LATCH && !force ) {
        if ( !this->latch.isEmpty()) {
            if ( !QString::compare( this->stringValue, string ))
                return true;
        }
        if ( QString::compare( this->latch, string ))
            this->latch = string;
        com.print( this->tr( " ^3'%1' will be changed upon restart\n" ).arg( this->name ));
    } else {
        if ( QString::compare( this->stringValue, string )) {
            this->stringValue = string;
            emit valueChanged( this->name, this->stringValue );
        }
    }
    return true;
}

/*
============
setInteger
============
*/
bool pCvar::setInteger( int integer ) {
    return this->set( QString( "%1" ).arg( integer ));
}

/*
============
setValue
============
*/
bool pCvar::setValue( float value ) {
    return this->set( QString( "%1" ).arg( value ));
}
