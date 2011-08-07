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

#ifndef MOD_CVARFUNC_H
#define MOD_CVARFUNC_H

//
// includes
//
#include "../common/sys_shared.h"
#include "../common/sys_cvarfunc.h"

//
// class::mCvar
//
class mCvar {
    Q_CLASSINFO( "description", "Module console variable" )
    Q_DISABLE_COPY( mCvar )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( QString string READ string WRITE setString )
    Q_FLAGS( Flags Flag )

public:
    // constructors/destructors
    mCvar( const QString &name, const QString &string, pCvar::Flags flags = pCvar::NoFlags, const QString &desc = QString::null );
    ~mCvar();

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    QString string() const { return this->m_string; }

    // flags & other funcs
    pCvar::Flags flags;

    // other funcs
    int     integer() const;
    float   value() const;
    bool    set( const QString &string, bool force = false );
    bool    set( int, bool force = false );
    bool    set( float, bool force = false );
    void    update( const QString &string );

public slots:
    // property setters
    void setName( const QString &cvarName ) { this->m_name = cvarName; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setString( const QString &string ) { this->m_string = string; }

private:
    // properties
    QString m_string;
    QString m_name;
    QString m_description;
};

#endif // MOD_CVARFUNC_H
