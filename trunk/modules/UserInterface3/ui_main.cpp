/*
===========================================================================
Copyright (C) 2011-2012 Edd 'Double Dee' Psycho

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
#include "ui_main.h"
#include "ui_script.h"
#include "ui_item.h"
#include "../mod_trap.h"
#include "../mod_script.h"
#include "../../common/sys_common.h"

//
// classes
//
class UiMain m;

/*
================
init
================
*/
void UiMain::init() {
    this->uiS = new UI_ScriptEngine();
}

/*
================
shutdown
================
*/
void UiMain::shutdown() {
    this->uiS->~UI_ScriptEngine();
}

/*
================
update
================
*/
void UiMain::update() {
    this->uiS->mse->update();
}

/*
================
render
================
*/
void UiMain::render() {
    // render assets
    foreach( UiItem *itemPtr, m.itemList ) {
        if ( !itemPtr->hasParent())
            itemPtr->render( itemPtr->context());
    }

    // update scripts
    if ( this->currentContext() == Default )
        m.update();
}

/*
================
modMain
================
*/
extern "C" MODULE_EXPORT QVariant modMain( ModuleAPI::ModuleAPICalls callNum, const QVariantList &args ) {
    switch ( callNum ) {
    case ModuleAPI::ModAPI:
        // return api version
        return ModuleAPI::Version;

    case ModuleAPI::Init:
        // check for renderer
        com.print( Sys::cYellow + QObject::trUtf8( "UI3: ^5checking for renderer\n" ));
        if ( !yp2.flags().testFlag( ModuleAPI::RendererLoaded )) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "UI3: no renderer detected, aborting initialization\n" ));
            return false;
        }

        // perform initialization
        com.print( Sys::cYellow + QObject::trUtf8( "UI3 ^5is running\n" ));

        // recache renderer assets
        r.reload();

        // init scripting
        m.init();
        break;


    case ModuleAPI::Update:
        // render background
        m.setBackground();
        m.render();

        // render default context
        m.setDefaultContext();
        m.render();

        // render foreground
        m.setForeground();
        m.render();
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
        com.print( Sys::cYellow + QObject::trUtf8( "UI3 is down\n" ));

        // shutdown
        m.shutdown();

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
        case ModuleAPI::KeyRelease:
            // emit signals if clicked
            foreach( UiItem *itemPtr, m.itemList ) {
                if ( !itemPtr->hasParent())
                    itemPtr->checkKeyEvent( static_cast<ModuleAPI::KeyEventType>( args.first().toInt()), args.at( 1 ).toInt());
            }
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

        // emit signals if clicked
        foreach( UiItem *itemPtr, m.itemList ) {
            if ( !itemPtr->hasParent())
                itemPtr->checkMouseEvent( static_cast<ModuleAPI::KeyEventType>( args.first().toInt()), args.at( 1 ).toInt());
        }

        break;

    case ModuleAPI::MouseMotion:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, QObject::trUtf8( "modMain: mouseMotion [x] [y]\n" ));
            return false;
        }
        // set current pos
        m.mousePos = QPoint( args.at( 0 ).toInt(), args.at( 1 ).toInt());

        // emit signals if mouseover
        foreach( UiItem *itemPtr, m.itemList ) {
            if ( !itemPtr->hasParent())
                itemPtr->checkMouseOver();
        }
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
extern "C" MODULE_EXPORT void modEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setPlatformCalls( reinterpret_cast<platformSyscallDef>( syscallPtr ));
}

/*
================
modRendererEntry
================
*/
extern "C" MODULE_EXPORT void modRendererEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setRendererCalls( reinterpret_cast<platformSyscallDef>( syscallPtr ));
}
