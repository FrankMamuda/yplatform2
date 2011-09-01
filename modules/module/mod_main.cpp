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
#include "mod_main.h"
#include "../mod_trap.h"

//
// classes
//
extern class Mod_Trap mt;

/*
================
construct
================
*/
Mod_Main::Mod_Main()
{
}

/*
================
modMain
================
*/
extern "C" MODULESHARED_EXPORT intptr_t modMain( ModuleAPI::ModuleAPICalls callNum, int numArgs, intptr_t *args ) {
    Q_UNUSED( numArgs );

    switch ( callNum ) {
    case ModuleAPI::ModAPI:
        // return api version
        return ModuleAPI::Version;

    case ModuleAPI::Init:
        // perform initialization
        mt.comPrint( "Hello world\n" );
        break;

    case ModuleAPI::Update:
        // perform updates (render something, etc.)
        break;

    case ModuleAPI::UpdateCvar:
        // console variable auto updater
        foreach ( mCvar *cvarPtr, mt.cvars ) {
            if ( !QString::compare( cvarPtr->name(), ( const char *)args[0] ))
                cvarPtr->update(( const char *)args[1] );
        }
        break;

    case ModuleAPI::Shutdown:
        // perform shutdown
        mt.comPrint( "Goodbye cruel world\n" );

        // clear cvars
        foreach ( mCvar *cvarPtr, mt.cvars )
            delete cvarPtr;
        break;
    }

    return true;
}

/*
================
modEntry
================
*/
extern "C" MODULESHARED_EXPORT void modEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setPlatformCalls(( platformSyscallDef )syscallPtr );
}

/*
================
modRendererEntry
================
*/
extern "C" MODULESHARED_EXPORT void modRendererEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setRendererCalls(( platformSyscallDef )syscallPtr );
}
