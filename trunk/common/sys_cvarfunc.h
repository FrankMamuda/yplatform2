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

#ifndef SYS_CVARFUNC_H
#define SYS_CVARFUNC_H

//
// includes
//
#include "sys_shared.h"

//
// class:pCvar
//
class pCvar : public QObject {
    Q_OBJECT
#ifndef MODULE_BUILD
    Q_CLASSINFO( "description", "Console variable" )
    //Q_DISABLE_COPY( pCvar )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( QString string READ string WRITE setString )
    Q_PROPERTY( QString reset READ resetString WRITE setResetString )
    Q_PROPERTY( QString latch READ latchString WRITE setLatchString )
#endif
    Q_FLAGS( Flags Flag )
    Q_FLAGS( AccessFlags AccessFlag )

public:
    // cvar flags
    enum Flag {
        NoFlags     = 0x0,
        Archive     = 0x1,
        Latched     = 0x2,
        ReadOnly    = 0x4,
        Password    = 0x8
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    // access flags
    enum AccessFlag {
        NoAccessFlags   = 0x0,
        Force           = 0x1,
        Temp            = 0x2,
        Config          = 0x4
    };
    Q_DECLARE_FLAGS( AccessFlags, AccessFlag )

#ifndef MODULE_BUILD
    Flags flags;
    
    // constructors/destructors
    pCvar( const QString &name, const QString &string, Flags flags = NoFlags, const QString &desc = QString::null, bool mCvar = false );
    pCvar() {}
    ~pCvar();

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    QString string( bool temp = false ) const { if ( !temp ) return this->m_string; else return this->m_temp; }
    QString resetString() const { return this->m_reset; }
    QString latchString() const { return this->m_latch; }

    // other funcs
    int     integer( bool = false ) const;
    float   value( bool = false ) const;
    bool    set( const QString &string, AccessFlags = NoAccessFlags );
    bool    set( int, AccessFlags = NoAccessFlags );
    bool    set( double, AccessFlags = NoAccessFlags );
    bool    set( float, AccessFlags = NoAccessFlags );
    void    reset();
    bool    passwordCheck();

public slots:
    // property setters
    void setName( const QString &cvarName ) { this->m_name = cvarName; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setString( const QString &string, bool temp = false ) { if ( !temp ) this->m_string = string; else this->m_temp = string; }
    void resetTempString() { this->m_temp = this->m_string; }
    void setTempString() { this->m_string = this->m_temp; emit valueChanged( this->name(), this->m_temp ); }
    void setResetString( const QString &string ) { this->m_reset = string; }
    void setLatchString( const QString &string = QString::null ) { this->m_latch = string; }

signals:
    void valueChanged( const QString &cvar, const QString &stringValue );

private:
    // properties
    QString m_string;
    QString m_temp;
    QString m_name;
    QString m_description;
    QString m_reset;
    QString m_latch;
#endif
};

// declare flags
Q_DECLARE_OPERATORS_FOR_FLAGS( pCvar::Flags )

#endif // SYS_CVARFUNC_H
