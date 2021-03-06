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
#include "../gui/gui_password.h"
#include "../applet/app_hasher.h"

//
// cvars
//
extern pCvar *sys_developer;
extern pCvar *sys_protected;
extern pCvar *sys_password;

/*
============
construct
============
*/
pCvar::pCvar( const QString &name, const QString &string, Flags flags, const QString &desc, Types type ) {
    // set the defaults
    this->flags = flags;
    this->setName( name );
    this->setString( string );
    this->setResetString( string );
    this->setLatchString();
    this->setDescription( desc );
    this->setType( type );
    this->setMinimum();
    this->setMaximum();

    // perform module cvar updates when needed (instead of reloading value on each frame)
    if ( flags.testFlag( pCvar::External ))
        this->connect( this, SIGNAL( valueChanged( QString, QString )), &mod, SLOT( updateCvar( QString, QString )));
}

/*
============
destruct
============
*/
pCvar::~pCvar() {
    this->disconnect( &mod, SLOT( updateCvar( QString, QString )));
}

/*
============
integer
============
*/
int pCvar::integer( bool temp ) const {
    bool valid;
    int y;

    // set integer if any
    y = this->string( temp ).toInt( &valid );

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
float pCvar::value( bool temp ) const {
    bool valid;
    float y;

    // set integer if any
    y = this->string( temp ).toFloat( &valid );

    // all ok, return float value
    if ( valid )
        return y;

    return 0;
}

/*
============
passwordCheck
============
*/
bool pCvar::passwordCheck() {
    if ( !this->flags.testFlag( Password ) || !sys_protected->isEnabled())
        return true;

    if ( sys_developer->isDisabled()) {
        Gui_Password p;
        if ( p.exec()) {
            if ( !QString::compare( hash.encrypt( p.ui->pwdEdit->text()), sys_password->string()))
                return true;
            else {
                com.print( Sys::cRed + this->tr( " \"%1\" is password protected\n" ).arg( this->name()));
                return false;
            }
        }

        return false;
    }
    return true;
}

/*
============
set
============
*/
bool pCvar::set( const QString &string, AccessFlags access ) {
    if ( !access.testFlag( Force ) && !this->passwordCheck())
        return false;

    if ( this->flags.testFlag( ReadOnly ) && !access.testFlag( Force )) {
        com.print( Sys::cRed + this->tr( " \"%1\" is read only\n" ).arg( this->name()));
    } else if ( this->flags.testFlag( Latched ) && !access.testFlag( Force )) {
        if ( !QString::compare( this->string(), string )) {
            this->m_latch.clear();
            return true;
        }

        if ( QString::compare( this->latchString(), string ))
            this->setLatchString( string );

        com.print( Sys::cYellow + this->tr( " \"%1\" will be changed upon restart\n" ).arg( this->name()));
    } else {
        QString tempStr( string );
        if ( !QString::compare( this->name(), "sys_password" ) && !access.testFlag( Config ))
             tempStr = hash.encrypt( string );

        if ( QString::compare( this->string(), tempStr )) {
            if ( access.testFlag( Temp ))
                this->setString( tempStr, true );
            else
                this->setString( tempStr, false );

            if ( !access.testFlag( Temp ))
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
bool pCvar::set( int integer, AccessFlags access ) {
    if ( this->type() == Integer && this->isClamped()) {
        if ( integer <= this->minimum().toInt())
            integer = this->minimum().toInt();

        if ( integer >= this->maximum().toInt())
            integer = this->maximum().toInt();
    }

    return this->set( QString( "%1" ).arg( integer ), access );
}

/*
============
set
============
*/
bool pCvar::set( bool value, AccessFlags access ) {
    if ( this->type() == Boolean && this->isClamped()) {
        if ( value <= false )
            value = false;

        if ( value >= true )
            value = true;
    }

    return this->set( static_cast<int>( value ), access );
}

/*
============
set
============
*/
bool pCvar::set( float value, AccessFlags access ) {
    if ( this->type() == Value && this->isClamped()) {
        if ( value <= this->minimum().toFloat())
            value = this->minimum().toFloat();

        if ( value >= this->maximum().toFloat())
            value = this->maximum().toFloat();
    }

    return this->set( QString( "%1" ).arg( value ), access );
}

/*
============
reset
============
*/
void pCvar::reset() {
    this->setString( this->resetString());
}

/*
============
setMinimum
============
*/
void pCvar::setMinimum( QVariant min ) {
    if ( this->type() == String )
        this->m_min = QVariant( 0 );
    else if ( this->type() == Boolean )
        this->m_min = QVariant( false );
    else
        this->m_min = min;
}

/*
============
setMaximum
============
*/
void pCvar::setMaximum( QVariant max ) {
    if ( this->type() == String )
        this->m_max = QVariant( 0 );
    else if ( this->type() == Boolean )
        this->m_max = QVariant( true );
    else
        this->m_max = max;
}

/*
============
isClamped
============
*/
bool pCvar::isClamped() const {
    if ( this->m_min == this->m_max || this->type() == String )
        return false;
    else
        return true;
}
