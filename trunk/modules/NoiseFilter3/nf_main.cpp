/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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
// Third reincarnation of Noise filter. Applet scans for new storage devices, mounts them,
// scans for noise and blocks screen if finds any.
//
// This applet is of beta quality, so don't expect it to behave perfectly. Also this will
// ONLY work on linux (and possibly other unices) with KDE's Solid installed (unless someone
// ports solid to windows, etc.
//

//
// includes
//
#include "nf_main.h"
#include "nf_devlist.h"
#include "nf_devenum.h"
#include "../mod_trap.h"
#include "../mod_cvarfunc.h"
#include "../../common/sys_common.h"

//
// cvars
//
mCvar *nf_blockableItems;

/*
================
modMain
================
*/
extern "C" MODULESHARED_EXPORT QVariant modMain( ModuleAPI::ModuleAPICalls callNum, const QVariantList &args ) {
    switch ( callNum ) {
    case ModuleAPI::ModAPI:
        // return api version
        return ModuleAPI::Version;

    case ModuleAPI::Init:
    {
        // perform initialization
        gui.clearConsole();
        com.print( Sys::cRed + QObject::trUtf8( "Noise smugglers, beware!\n" ), 16 );

        // load blockable items
        nf_blockableItems = cv.create( "nf_blockableItems", "benny;beny;benasi;bennasi;bennassi;benassi;satisfaction;skrillex;skrilex;dubstep", pCvar::Archive );

        // initialize NoiseFilter
        de.init();
        gui.createSystemTray();
        gui.hide();
    }
        break;

    case ModuleAPI::Update:
        // perform updates (render something, etc.)
        break;

    case ModuleAPI::UpdateCvar:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: updateCvar [cvarName] [string]\n" ));
            return false;
        }

        // console variable auto updater
        cv.update( args.at( 0 ).toString(), args.at( 1 ).toString());
        break;

    case ModuleAPI::Shutdown:
        // perform shutdown
        de.shutdown();
        gui.removeSystemTray();

        // clear cvars
        cv.clear();
        break;

    case ModuleAPI::KeyEvent:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: keyEvent [type] [key]\n" ));
            return false;
        }

        switch ( static_cast<ModuleAPI::KeyEventType>( args.first().toInt())) {
        case ModuleAPI::KeyPress:
            // do smth with this data
            break;

        case ModuleAPI::KeyRelease:
            // do smth with this data
            break;

        default:
        case ModuleAPI::DoubleClick:
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: invalid keyEvent type '%1'\n" ).arg( static_cast<ModuleAPI::KeyEventType>( args.first().toInt())));
            return false;
        }
        break;

    case ModuleAPI::MouseEvent:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: mouseEvent [type] [key]\n" ));
            return false;
        }

        switch ( static_cast<ModuleAPI::KeyEventType>( args.first().toInt())) {
        case ModuleAPI::KeyPress:
            // do smth with this data
            break;

        case ModuleAPI::KeyRelease:
            // do smth with this data
            break;

        case ModuleAPI::DoubleClick:
            // do smth with this data
            break;

        default:
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: invalid keyEvent type '%1'\n" ).arg( static_cast<ModuleAPI::KeyEventType>( args.first().toInt())));
            return false;
        }
        break;

    case ModuleAPI::MouseMotion:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: mouseMotion [x] [y]\n" ));
            return false;
        }
        // do smth with this data
        break;

    case ModuleAPI::WheelEvent:
        // do smth with this data
        break;

    default:
        com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: unknown callNum %1\n" ).arg( callNum ));
        return false;
    }

    return true;
}

/*
================
modEntry
================
*/
extern "C" MODULESHARED_EXPORT void modEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setPlatformCalls( reinterpret_cast<platformSyscallDef>( syscallPtr ));
}

/*
================
modRendererEntry
================
*/
extern "C" MODULESHARED_EXPORT void modRendererEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setRendererCalls( reinterpret_cast<platformSyscallDef>( syscallPtr ));
}
