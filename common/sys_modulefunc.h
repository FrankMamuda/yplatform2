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

#ifndef SYS_MODULEFUNC_H
#define SYS_MODULEFUNC_H

//
// includes
//
#include "sys_shared.h"

//
// defines
//
#if !defined( intptr_t )
#include <stdint.h>
#endif

//
// prototypes for resolvable functions
//
typedef intptr_t ( *modMainDef )( int, int, intptr_t * );
typedef void ( *modEntryDef )( intptr_t (*)( int, int, intptr_t * ));

//
// class::pModule
//
class pModule : public QObject {
    Q_OBJECT

public:
    // module info
    QString name;
    QString description;
    bool renderer;
    bool postInit;
    QString versionString;
    int apiVersion;
    QString icon;
    QString filename;
    QString errorMessage;
    QString manifestName;
    bool loaded;

    // constructor
    pModule( const QString &moduleName );

    // calls
    intptr_t call( int ) const;
    intptr_t call( int, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;

private:
    void loadHandle();

    // module libHandle and funcs
    QLibrary *handle;
    modMainDef modMain;
    modEntryDef entry;

public slots:
    void update();
    void unload();
    void load();
};

//
// inlines for module calls
//
inline intptr_t pModule::call( int callNum ) const { return this->modMain( callNum, 0, NULL ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0 ) const { intptr_t args[1] = { arg0 }; return this->modMain( callNum, 1, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1 ) const { intptr_t args[2] = { arg0, arg1 }; return this->modMain( callNum, 2, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2 ) const { intptr_t args[3] = { arg0, arg1, arg2 }; return this->modMain( callNum, 3, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3 ) const { intptr_t args[4] = { arg0, arg1, arg2, arg3 }; return this->modMain( callNum, 4, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4 ) const { intptr_t args[5] = { arg0, arg1, arg2, arg3, arg4 }; return this->modMain( callNum, 5, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4, const intptr_t arg5 ) const { intptr_t args[6] = { arg0, arg1, arg2, arg3, arg4, arg5 }; return this->modMain( callNum, 6, args ); }

#endif // SYS_MODULEFUNC_H
