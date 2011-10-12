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

/*
============
construct
============
*/
mCvar::mCvar( const QString &name, const QString &string, pCvar::Flags flags, const QString &desc ) {
    // set the defaults
    this->flags = flags;
    this->setName( name );
    this->setString( string );
    this->setDescription( desc );
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
integer
============
*/
int mCvar::integer() const {
    bool valid;
    int y;

    // set integer if any
    y = this->string().toInt( &valid );

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
float mCvar::value() const {
    bool valid;
    float y;

    // set integer if any
    y = this->string().toFloat( &valid );

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
    bool ret;

    // call platform
    ret = mt.cvarSet( this->name(), string, force );

    // set local cvar
    this->setString( mt.cvarGet( this->name()));

    // return result
    return ret;
}

/*
============
update
============
*/
void mCvar::update( const QString &string ) {
    if ( QString::compare( this->string(), string ))
        this->setString( string );
}

/*
============
setString
============
*/
void mCvar::setString( const QString &string ) {
    this->m_string = string;

    // emit signal
    emit this->valueChanged( this->name(), this->string());
}

/*
============
set (integer)
============
*/
bool mCvar::set( int integer, bool force ) {
    return this->set( QString( "%1" ).arg( integer ), force );
}

/*
============
set (value)
============
*/
bool mCvar::set( float value, bool force ) {
    return this->set( QString( "%1" ).arg( value ), force );
}
