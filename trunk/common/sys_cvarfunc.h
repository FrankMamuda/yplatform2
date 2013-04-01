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
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( QString string READ string WRITE setString )
    Q_PROPERTY( QString reset READ resetString WRITE setResetString )
    Q_PROPERTY( QString latch READ latchString WRITE setLatchString )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( QRegExp pattern READ pattern WRITE setPattern RESET resetPattern )
    Q_PROPERTY( QVariant minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( QVariant maximum READ maximum WRITE setMaximum )
#endif
    Q_FLAGS( Flags Flag )
    Q_FLAGS( AccessFlags AccessFlag )

public:
    // cvar flags
    enum Flag {
        NoFlags     = 0x00,
        Archive     = 0x01,
        Latched     = 0x02,
        ReadOnly    = 0x04,
        Password    = 0x08,
        External    = 0x10 /* as in module cvar */
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    // cvar type flags
    enum Types {
        String      = 0,
        Integer     = 1,
        Value       = 2,
        Boolean     = 3
    };
    Q_ENUMS( Types )

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
    pCvar( const QString &name, const QString &string, Flags flags = NoFlags, const QString &desc = QString::null, Types type = String );
    pCvar() {}
    ~pCvar();

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    QString string( bool temp = false ) const { if ( !temp ) return this->m_string; else return this->m_temp; }
    QString resetString() const { return this->m_reset; }
    QString latchString() const { return this->m_latch; }
    Types type() const { return this->m_type; }
    QRegExp pattern() const { return this->m_pattern; }

    // other funcs
    int     integer( bool = false ) const;
    bool    boolean( bool temp = false ) const { return this->integer( temp ); }
    bool    isEnabled() { return this->boolean(); }
    bool    isDisabled() { return !this->boolean(); }
    float   value( bool = false ) const;
    bool    set( const QString &string, AccessFlags = NoAccessFlags );
    bool    set( int, AccessFlags = NoAccessFlags );
    bool    set( bool, AccessFlags = NoAccessFlags );
    bool    set( float, AccessFlags = NoAccessFlags );
    void    reset();
    bool    passwordCheck();

    // clamp values (QVariant should work for all, cannot use templates)
    void setMinimum( QVariant min = QVariant( false ));
    void setMaximum( QVariant max = QVariant( false ));
    QVariant minimum() const { return this->m_min; }
    QVariant maximum() const { return this->m_max; }
    bool isClamped() const;

public slots:
    // property setters
    void setName( const QString &cvarName ) { this->m_name = cvarName; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setString( const QString &string, bool temp = false ) { if ( !temp ) this->m_string = string; else this->m_temp = string; }
    void resetTempString() { this->m_temp = this->m_string; }
    void setTempString() { this->m_string = this->m_temp; emit valueChanged( this->name(), this->m_temp ); }
    void setResetString( const QString &string ) { this->m_reset = string; }
    void setLatchString( const QString &string = QString::null ) { this->m_latch = string; }
    void setType( Types type = String ) { this->m_type = type; }
    void setPattern( const QRegExp &pattern ) { this->m_pattern = pattern; }
    void setPattern( const QString &pattern ) { this->m_pattern = QRegExp( pattern ); }
    void resetPattern() { this->m_pattern = QRegExp(); }

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
    Types m_type;
    QRegExp m_pattern;
    QVariant m_min;
    QVariant m_max;
#endif
};

// declare flags
Q_DECLARE_OPERATORS_FOR_FLAGS( pCvar::Flags )

#endif // SYS_CVARFUNC_H
