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

public:
    mCvar( const QString &name, const QString &string, int flags = 0, const QString &desc = QString::null );
    ~mCvar();
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
    void    update( const QString &string );
};

#endif // MOD_CVARFUNC_H
