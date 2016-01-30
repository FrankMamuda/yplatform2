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
#include "r_public.h"
#include "r_main.h"
#include "r_cmd.h"
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
extern "C" RENDERER_EXPORT void modEntry( intptr_t ( *syscallPtr )( int, intptr_t, ... )) {
    mt.setPlatformCalls(( platformSyscallDef )syscallPtr );
}

/*
================
modMain
================
*/
extern "C" RENDERER_EXPORT QVariant rendererMain( RendererAPI::RendererAPICalls callNum, const QVariantList &args ) {
    switch ( callNum ) {
    case RendererAPI::ModAPI:
        // return api version
        return static_cast<int>( RendererAPI::Version );

    case RendererAPI::Init:
        // perform initialization
        m.init();

        return m.hasInitialized();


    case RendererAPI::UpdateCvar:
        if ( !m.hasInitialized() )
            return false;

        // console variable auto updater
        cv.update( args.at( 0 ).toString(), args.at( 1 ).toString());
        break;

    case RendererAPI::Shutdown:
        if ( !m.hasInitialized())
            return false;

        // perform shutdown
        m.shutdown();

        // clear cvars
        cv.clear();
        break;

    case RendererAPI::BeginFrame:
        if ( !m.hasInitialized())
            return false;

        m.renderer->begin();
        break;

    case RendererAPI::EndFrame:
        m.renderer->end();
        break;

    case RendererAPI::LoadMaterial:
        if ( !m.hasInitialized())
            return false;

        return static_cast<int>( m.loadMaterial( args.first().toString()));

    case RendererAPI::DrawMaterial:
        if ( !m.hasInitialized())
            return false;

        rCmd.drawMaterial( args.at( 0 ).toFloat(), args.at( 1 ).toFloat(), args.at( 2 ).toFloat(), args.at( 3 ).toFloat(),
                           static_cast<mtrHandle_t>( args.at( 4 ).toInt()));
        break;

    case RendererAPI::DrawText:
#if 0
        /*if ( !m.hasInitialized())
            return false;

        // save colour, set new font colour
        rCmd.saveColour();
        rCmd.setColour( args.at( 4 ).toFloat(), args.at( 5 ).toFloat(), args.at( 6 ).toFloat(), args.at( 7 ).toFloat(), true );

        // draw Qt font
        glImp.drawText( args.at( 0 ).toFloat(), args.at( 1 ).toFloat(), args.at( 2 ).value<QFont>(), args.at( 3 ).toString());

        // restore colour
        rCmd.restoreColour();*/
#endif
        break;

    case RendererAPI::SetColour:
        if ( !m.hasInitialized())
            return false;

        rCmd.setColour( args.at( 0 ).toFloat(), args.at( 1 ).toFloat(), args.at( 2 ).toFloat(), args.at( 3 ).toFloat());
        break;

    case RendererAPI::Raise:
        if ( !m.hasInitialized())
            return false;

        m.renderer->show();
        break;

    case RendererAPI::Hide:
        if ( !m.hasInitialized())
            return false;

        m.renderer->hide();
        break;

    case RendererAPI::State:
        if ( !m.hasInitialized())
            return RendererAPI::Hidden;

        if ( m.renderer->isVisible())
            return static_cast<int>( RendererAPI::Raised );
        else
            return static_cast<int>( RendererAPI::Hidden );

    case RendererAPI::Reload:
        m.shutdown( true );
        m.init( true );
        break;

    case RendererAPI::SetWindowTitle:
        m.renderer->setTitle( args.at( 0 ).toString());
        break;
    }

    return true;
}
