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

#ifndef SYS_CVARFUNC_H
#define SYS_CVARFUNC_H

//
// includes
//
#include "sys_shared.h"

//
// defines
//
#define CVAR_ARCHIVE    1
#define CVAR_LATCH      2
#define CVAR_ROM        4
#define CVAR_USER       16
#define CVAR_PASSWORD   64
#define CVAR_NULL       0xFFFFFFFF

//
// class::pCvar
//
class pCvar : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Console variable" )
    Q_DISABLE_COPY( pCvar )

public:
    QString reset;
    QString latch;
    pCvar( const QString &name, const QString &string, int flags = 0, const QString &desc = QString::null, bool mCvar = false );
    ~pCvar();
    QString name;
    QString stringValue;
    QString string();
    QString description;
    int     flags;
    int     integer();
    bool    set( const QString &string, bool force = false );
    float   value();
    bool    setInteger( int );
    bool    setValue( float );

signals:
    void valueChanged( const QString &cvar, const QString &stringValue );
};

#endif // SYS_CVARFUNC_H
