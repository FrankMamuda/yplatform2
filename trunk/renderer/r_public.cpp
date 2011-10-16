/*
===========================================================================
Copyright (C) 2011 Edd 'Double Dee' Psycho

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
#include "r_public.h"
#include "r_main.h"
#include "r_cmd.h"
#include "r_glimp.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

//
// cvars
//
extern mCvar *r_screenMode;
extern mCvar *r_adjustScreen;

/*
================
modEntry
================
*/
extern "C" RENDERERSHARED_EXPORT void modEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setPlatformCalls(( platformSyscallDef )syscallPtr );
}

/*
================
modMain
================
*/
extern "C" RENDERERSHARED_EXPORT intptr_t rendererMain( RendererAPI::RendererAPICalls callNum, int numArgs, intptr_t *args ) {
    Q_UNUSED( numArgs );

    switch ( callNum ) {
    case RendererAPI::ModAPI:
        // return api version
        return static_cast<intptr_t>( RendererAPI::Version );

    case RendererAPI::Init:
        // perform initialization
        m.init();

        return m.hasInitialized();

    case RendererAPI::UpdateCvar:
        if ( !m.hasInitialized() )
            return false;

        // console variable auto updater
        foreach ( mCvar *cvarPtr, mt.cvars ) {
            if ( !QString::compare( cvarPtr->name(), QString::fromLatin1( reinterpret_cast<const char*>( args[0] ))))
                cvarPtr->update( QString::fromLatin1( reinterpret_cast<const char*>( args[1] )));
        }
        break;

    case RendererAPI::Shutdown:
        if ( !m.hasInitialized())
            return false;

        // perform shutdown
        m.shutdown();

        // clear cvars
        foreach ( mCvar *cvarPtr, mt.cvars )
            delete cvarPtr;
        break;

    case RendererAPI::BeginFrame:
        if ( !m.hasInitialized())
            return false;

        // begin frame
        m.beginFrame();

        // draw platform logo
        cmd.setColour( Renderer::ColourWhite );
        if ( r_adjustScreen->integer())
            cmd.drawMaterial( Renderer::HorizontalScreenModes[Renderer::DefaultScreenMode]/2-256/2,
                              Renderer::VerticalScreenModes[Renderer::DefaultScreenMode]/2-256/2,
                              256, 256, m.platformLogo );
        else
            cmd.drawMaterial( Renderer::HorizontalScreenModes[glImp.getScreenMode()]/2-256/2,
                              Renderer::VerticalScreenModes[glImp.getScreenMode()]/2-256/2,
                              256, 256, m.platformLogo );
        break;

    case RendererAPI::EndFrame:
        if ( !m.hasInitialized())
            return false;

        // reset colour to null
        cmd.setColour();

        // end frame
        m.endFrame();
        break;

    case RendererAPI::LoadMaterial:
        if ( !m.hasInitialized())
            return false;

        return static_cast<intptr_t>( m.loadMaterial( QString::fromLatin1( reinterpret_cast<const char*>( args[0] ))));

    case RendererAPI::DrawMaterial:
        if ( !m.hasInitialized())
            return false;

        cmd.drawMaterial( mt.getFloat( args[0] ), mt.getFloat( args[1] ), mt.getFloat( args[2] ), mt.getFloat( args[3] ),
                          static_cast<mtrHandle_t>( args[4] ));
        break;

    case RendererAPI::DrawText:
        if ( !m.hasInitialized())
            return false;

        // save colour, set new font colour
        cmd.saveColour();
        cmd.setColour( mt.getFloat( args[4] ), mt.getFloat( args[5] ), mt.getFloat( args[6] ), mt.getFloat( args[7] ), true );

        // draw Qt font
        glImp.drawText( mt.getFloat( args[0] ), mt.getFloat( args[1] ), *( reinterpret_cast<QFont*>( args[2] )), QString::fromLatin1( reinterpret_cast<const char*>( args[3] )));

        // restore colour
        cmd.restoreColour();

        break;

    case RendererAPI::SetColour:
        if ( !m.hasInitialized())
            return false;

        cmd.setColour( mt.getFloat( args[0] ), mt.getFloat( args[1] ), mt.getFloat( args[2] ), mt.getFloat( args[3] ));
        break;

    case RendererAPI::Raise:
        if ( !m.hasInitialized())
            return false;

        glImp.widget->show();
        break;

    case RendererAPI::Hide:
        if ( !m.hasInitialized())
            return false;

        glImp.widget->hide();
        break;

    case RendererAPI::State:
        if ( !m.hasInitialized())
            return RendererAPI::Hidden;

        if ( glImp.widget->isVisible())
            return static_cast<intptr_t>( RendererAPI::Raised );
        else
            return static_cast<intptr_t>( RendererAPI::Hidden );
    }

    return true;
}
