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
#include "sys_cvarfunc.h"
#include "sys_cvar.h"
#include "sys_common.h"
#include "sys_module.h"

//
//  move slots to private?
//

/*
============
construct
============
*/
pCvar::pCvar( const QString &name, const QString &string, Flags flags, const QString &desc, bool mCvar ) {
    // set the defaults
    this->flags = flags;
    this->setName( name );
    this->setString( string );
    this->setResetString( string );
    this->setLatchString();
    this->setDescription( desc );

    // perform module cvar updates when needed (instead of reloading value on each frame)
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
integer
============
*/
int pCvar::integer() const {
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
float pCvar::value() const {
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
bool pCvar::set( const QString &string, bool force ) {
    if ( this->flags.testFlag( ReadOnly ) && !force ) {
        com.print( this->tr( " ^1'%1' is read only\n" ).arg( this->name()));
#if 0
        // disabled for now
        // to be enabled when Gui_Settings is complete
    } else if ( this->flags.testFlag( Password ) && !com.developerMode ) {
        com.print( this->tr( " ^1'%1' is password protected\n" ).arg( this->name()));
#endif
    } else if ( this->flags.testFlag( Latched ) && !force ) {
        if ( !this->latchString().isEmpty()) {
            if ( !QString::compare( this->string(), string ))
                return true;
        }
        if ( QString::compare( this->latchString(), string ))
            this->setLatchString( string );
        com.print( this->tr( " ^3'%1' will be changed upon restart\n" ).arg( this->name()));
    } else {
        if ( QString::compare( this->string(), string )) {
            this->setString( string );
            emit valueChanged( this->name(), this->string());
        }
    }
    return true;
}

/*
============
set
============
*/
bool pCvar::set( int integer, bool force ) {
    return this->set( QString( "%1" ).arg( integer ), force );
}

/*
============
set
============
*/
bool pCvar::set( double integer, bool force ) {
    return this->set( QString( "%1" ).arg( integer ), force );
}

/*
============
set
============
*/
bool pCvar::set( float value, bool force ) {
    return this->set( QString( "%1" ).arg( value ), force );
}

/*
============
reset
============
*/
void pCvar::reset() {
    this->setString( this->resetString());
}
