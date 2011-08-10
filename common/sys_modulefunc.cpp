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
// cvars
//
extern pCvar *mod_extract;

/*
================
construct
================
*/
pModule::pModule( const QString &moduleName ) {
    this->setLoaded( false );
    this->setName( moduleName );
    this->setType( Module );
}

/*
=================
platformSyscalls
=================
*/
intptr_t platformSyscalls( ModuleAPI::PlatformAPICalls callNum, int numArgs, intptr_t *args ) {
    return mod.platformSyscalls( callNum, numArgs, args );
}

/*
=================
rendererSyscalls
=================
*/
intptr_t rendererSyscalls( RendererAPI::RendererAPICalls callNum, int numArgs, intptr_t *args ) {
    return mod.rendererSyscalls( callNum, numArgs, args );
}

/*
================
unload
================
*/
void pModule::unload() {
    if ( this->handle != NULL ) {
        if ( this->type() == Renderer )
            this->call( RendererAPI::Shutdown );
        else
            this->call( ModuleAPI::Shutdown );

        // dd, it seems like we cannot perform unload, it crashes for some reason
        //this->handle->unload();
        delete this->handle;
    }
}

/*
================
update
================
*/
void pModule::update() {
    if ( this->type() != Renderer )
        this->call( ModuleAPI::Update );
}

/*
================
loadHandle
================
*/
void pModule::loadHandle() {
    // attempt loading
    if ( this->handle->load()) {
        // resolve main entry (renderer uses different structure)
        if ( this->type() == Module ) {
            this->modMain = ( modMainDef )this->handle->resolve( "modMain" );
            this->renderer = ( rendererEntryDef )this->handle->resolve( "modRendererEntry" );
        } else if ( this->type() == Renderer )
            this->modMain = ( modMainDef )this->handle->resolve( "rendererMain" );

        // resolve syscall entry
        this->entry = ( modEntryDef )this->handle->resolve( "modEntry" );

        // see if we have properly resolved entry/main functions
        if ( !this->modMain || !this->entry ) {
            this->setErrorMessage( this->tr( "could not load module: errorString: %1" ).arg( this->handle->errorString()));
            this->handle->unload();
            delete this->handle;
        } else {
            // pass platform syscalls
            this->entry( platformSyscalls );

            // pass renderer calls if needed
            if ( this->type() == Module && this->renderer )
                this->renderer( rendererSyscalls );

            // produce call to the module
            unsigned int version = 0;
            if ( this->type() == Renderer )
                version = this->call( RendererAPI::ModAPI );
            else
                version = this->call( ModuleAPI::ModAPI );

            if ( this->type() == Module ) {
                if ( version > ModuleAPI::Version ) {
                    this->setErrorMessage( this->tr( "Module API version mismatch - %1, expected less or equal to %2" ).arg( version ).arg( ModuleAPI::Version ));
                    delete this->handle;
                    return;
                } else
                    com.print( this->tr( "^2pModule::loadHandle: successfully loaded module \"%1\" with API - %2\n" ).arg( this->name()).arg( version ));
            } else if ( this->type() == Renderer ) {
                if ( version > RendererAPI::Version ) {
                    this->setErrorMessage( this->tr( "Renderer API version mismatch - %1, expected less or equal to %2" ).arg( version ).arg( RendererAPI::Version ));
                    delete this->handle;
                    return;
                } else
                    com.print( this->tr( "^2pModule::loadHandle: successfully loaded renderer with API - %1\n" ).arg( version ));
            }

            // perform initialization
            bool mInit;
            if ( this->type() == Renderer ) {
                mInit = this->call( RendererAPI::Init );
            } else
                mInit = this->call( ModuleAPI::Init );

            if ( mInit ) {
                // success
                return;
            } else {
                if ( this->type() == Renderer )
                    this->setErrorMessage( this->tr( "Module could be initialized" ));
                else if ( this->type() == Renderer )
                    this->setErrorMessage( this->tr( "Renderer could be initialized" ));

                delete this->handle;
            }
        }
    } else {
        this->setErrorMessage( this->tr( "%1" ).arg( this->handle->errorString()));
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
    if ( this->filename().isEmpty())
        modName = this->name();
    else
        modName = this->filename();

    QString filename = QString( "modules/%1%2_%3_%4.%5" )
            .arg( LIBRARY_PREFIX )
            .arg( modName )
            .arg( LIBRARY_SUFFIX )
            .arg( ARCH_STRING )
            .arg( LIBRARY_EXT );

    // check if it exists in package and copy it (if we allow it)
    if ( mod_extract->integer()) {
        Sys_Filesystem::OpenFlags flags = Sys_Filesystem::Silent;
        QString mName = filename;
        if ( fs.exists( mName, flags, searchPathIndex )) {
            if (!( flags.testFlag( Sys_Filesystem::Linked ))) {
                pSearchPath *sp = fs.searchPaths.at( searchPathIndex );
                if ( sp->type() == pFile::Package )
                    fs.extract( filename );
            }
        } else {
            this->setErrorMessage( this->tr( "Could not find module \"%1\"\n" ).arg( modName ));
            return;
        }
    }

    // check if it exists (need to double check since it could be extracted)
    Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags;
    if ( fs.exists( filename, flags, searchPathIndex )) {
        if ( !( flags.testFlag( Sys_Filesystem::Linked ))) {
            pSearchPath *sp = fs.searchPaths.at( searchPathIndex );
            if ( sp->type() == pFile::Directory ) {
                // allocate library handle
                this->handle = new QLibrary( sp->path() + filename );
                this->loadHandle();
            }
        } else {
            // by now we already have the link target stored in filename
            this->handle = new QLibrary( filename );
            this->loadHandle();
        }
    } else {
        this->setErrorMessage( this->tr( "Could not load module" ));
        return;
    }
}
