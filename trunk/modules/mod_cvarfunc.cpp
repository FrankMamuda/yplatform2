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
#include "mod_cvarfunc.h"
#include "mod_trap.h"

//
// classes
//
extern class Mod_Trap mt;

/*
============
construct
============
*/
mCvar::mCvar( const QString &name, const QString &string, int flags, const QString &desc ) {
    // set the defaults
    this->flags = flags;
    this->name = name;
    this->stringValue = string;
    this->description = desc;
}

/*
============
destruct
============
*/
mCvar::~mCvar() {
}

/*
============
string
============
*/
QString mCvar::string() {
    return this->stringValue;
}

/*
============
integer
============
*/
int mCvar::integer() {
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
float mCvar::value() {
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
bool mCvar::set( const QString &string, bool force ) {
    // call platform
    return mt.cvarSet( this->name, string, force );
}


/*
============
update
============
*/
void mCvar::update( const QString &string ) {
    this->stringValue = string;
}

/*
============
setInteger
============
*/
bool mCvar::setInteger( int integer ) {
    return this->set( QString( "%1" ).arg( integer ));
}

/*
============
setValue
============
*/
bool mCvar::setValue( float value ) {
    return this->set( QString( "%1" ).arg( value ));
}
