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
#include "mod_trap.h"
#include "../common/sys_common.h"

//
// classes
//
class Mod_Trap mt;
class Mod_Common com;
class Mod_Filesystem fs;
class Mod_Cmd cmd;
class Mod_Cvar cv;
class Mod_App app;
class Mod_Gui gui;
#ifdef RENDERER_ENABLED
extern class Mod_Renderer r;
#endif

/*
================
setPlatformCalls
================
*/
void Mod_Trap::setPlatformCalls( platformSyscallDef pSysCall ) {
    this->platformSyscall = pSysCall;
}

#ifndef R_BUILD
/*
================
setRendererCalls
================
*/
void Mod_Trap::setRendererCalls( platformSyscallDef rSysCall ) {
    this->rendererSyscall = rSysCall;
}
#endif

/*
================
callExt
================
*/
QVariant Mod_Trap::callExt( ModuleAPI::Destination destination, int callNum, const QVariantList &args ) {
    switch ( destination ) {
    case ModuleAPI::Platform:
        return this->platformSyscall( callNum, args );
        break;
    case ModuleAPI::Renderer:
#ifndef R_BUILD
        return this->rendererSyscall( callNum, args );
#endif
        break;
    }
    return false;
}

/*
=============
listFiles
=============
*/
QStringList Mod_Filesystem::listFiles( const QString &directory, const QRegExp &filter, Sys_Filesystem::ListModes mode ) {
    QStringList list = mt.call( ModuleAPI::Platform, ModuleAPI::FsList, directory, filter, mode ).toStringList();

    // remove void entries
    list.removeAll( "" );
    return list;
}

//
// syscalls::cvar subsystem
//

/*
=============
create
=============
*/
mCvar *Mod_Cvar::create( const QString &name, const QString &string, pCvar::Flags flags, const QString &desc ) {
    bool ok = mt.call( ModuleAPI::Platform, ModuleAPI::CvarCreate, name, string, static_cast<int>( flags ), desc ).toBool();

    if ( ok ) {
        this->cvarList << new mCvar( name, cv.get( name ), flags, desc );
        return this->cvarList.last();
    }
    return NULL;
}

/*
=============
update
=============
*/
void Mod_Cvar::update( const QString &cvarName, const QString &string ) {
    foreach ( mCvar *cvarPtr, this->cvarList ) {
        if ( !QString::compare( cvarPtr->name(), cvarName ))
            cvarPtr->update( string );
    }
}

/*
=============
clear
=============
*/
void Mod_Cvar::clear() {
    // clear cvars
    foreach ( mCvar *cvarPtr, this->cvarList )
        delete cvarPtr;
}

/*
============
find
============
*/
mCvar *Mod_Cvar::find( const QString &name ) const {
    foreach ( mCvar *cvarPtr, this->cvarList ) {
        if ( !QString::compare( name, cvarPtr->name(), Qt::CaseInsensitive ))
            return cvarPtr;
    }
    return NULL;
}

#ifdef R_BUILD
/*
=============
keyEvent
=============
*/
void Mod_Trap::keyEvent( ModuleAPI::KeyEventType type, int key ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererKeyEvent, static_cast<int>( type ), static_cast<int>( key ));
}

/*
=============
mouseEvent
=============
*/
void Mod_Trap::mouseEvent( ModuleAPI::KeyEventType type, Qt::MouseButton key ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererMouseEvent, static_cast<int>( type ), static_cast<int>( key ));
}

/*
=============
mouseMotion
=============
*/
void Mod_Trap::mouseMotion( int x, int y ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererMouseMotion, static_cast<int>( x ), static_cast<int>( y ));
}

/*
=============
mouseMotion
=============
*/
void Mod_Trap::wheelEvent( int delta ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererWheelEvent, static_cast<int>( delta ));
}
#endif
