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
#include "sys_modulefunc.h"
#include "sys_common.h"
#include "sys_filesystem.h"
#include "sys_cvar.h"
#include "sys_module.h"

//
// classes
//
extern class Sys_Common com;
extern class Sys_Filesystem fs;
extern class Sys_Module mod;

//
// cvars
//
extern pCvar *mod_extract;

/*
================
construct
================
*/
pModule::pModule( const QString &moduleName ) {
    this->loaded = false;
    this->renderer = false;
    this->postInit = false;
    this->name = moduleName;
}

/*
=================
platformSyscalls
=================
*/
intptr_t platformSyscalls( int callNum, int numArgs, intptr_t *args ) {
    return mod.platformSyscalls( callNum, numArgs, args );
}

/*
================
unload
================
*/
void pModule::unload() {
    if ( this->handle != NULL ) {
        this->call( MOD_SHUTDOWN );
        this->handle->unload();
        delete this->handle;
    }
}

/*
================
update
================
*/
void pModule::update() {
    this->call( MOD_UPDATE );
}

/*
================
loadHandle
================
*/
void pModule::loadHandle() {
    // attempt loading
    if ( this->handle->load()) {
        this->modMain = ( modMainDef )this->handle->resolve( "modMain" );
        this->entry = ( modEntryDef )this->handle->resolve( "modEntry" );

        // see if we have properly resolved entry/main functions
        if ( !this->modMain || !this->entry ) {
            this->errorMessage = this->tr( "could not load module: errorString: %1" ).arg( this->handle->errorString());
            this->handle->unload();
            delete this->handle;
        } else {
            // pass platform syscalls
            this->entry( platformSyscalls );

            // produce call to the module
            int version = this->call( MOD_API );
            if ( version > MODULE_API_VERSION )
                this->errorMessage = this->tr( "API version mismatch - %1, expected less or equal to %2" ).arg( version ).arg( MODULE_API_VERSION );
            else {
                com.print( this->tr( "^2Sys_Module::loadHandle: successfully loaded module \"%1\" with API - %2\n" ).arg( this->name ).arg( version ));

                // perform initialization
                if ( this->call( MOD_INIT )) {
                    //this->modList << this;

                    // success
                    return;
                } else {
                    this->errorMessage = this->tr( "Module could be initialized" );
                    delete this->handle;
                }
            }
        }
    } else {
        this->errorMessage = this->tr( "%1" ).arg( this->handle->errorString());
        delete this->handle;
    }
}

/*
================
load
================
*/
void pModule::load() {
    int searchPathIndex;
    QString modName;

    // generate filename
    if ( this->filename.isEmpty())
        modName = this->name;
    else
        modName = this->filename;

    QString filename = QString( "modules/%1%2_%3_%4.%5" )
            .arg( LIBRARY_PREFIX )
            .arg( modName )
            .arg( LIBRARY_SUFFIX )
            .arg( ARCH_STRING )
            .arg( LIBRARY_EXT );

    // check if it exists in package and copy it (if we allow it)
    if ( mod_extract->integer()) {
        int flags = FS_FLAGS_SILENT;
        QString mName = filename;
        if ( fs.fileExists( mName, flags, searchPathIndex )) {
            if (!( flags & FS_FLAGS_LINKED )) {
                searchPath_t *sp = fs.searchPaths.at( searchPathIndex );
                if ( sp->type == SEARCHPATH_PACKAGE )
                    fs.extractFromPackage( filename );
            }
        } else {
            this->errorMessage = this->tr( "Could not find module" );
            return;
        }
    }

    // check if it exists (need to double check since it could be extracted)
    int flags = 0;
    if ( fs.fileExists( filename, flags, searchPathIndex )) {
        if ( !( flags & FS_FLAGS_LINKED )) {
            searchPath_t *sp = fs.searchPaths.at( searchPathIndex );
            if ( sp->type == SEARCHPATH_DIRECTORY ) {
                // allocate library handle
                this->handle = new QLibrary( sp->path + filename );
                this->loadHandle();
            }
        } else {
            // by now we already have the link target stored in filename
            this->handle = new QLibrary( filename );
            this->loadHandle();
        }
    } else {
        this->errorMessage = this->tr( "Could not load module" );
        return;
    }
}

