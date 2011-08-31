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
        return RendererAPI::Version;

    case RendererAPI::Init:
        // perform initialization
        m.init();

        return m.hasInitialized();

    case RendererAPI::UpdateCvar:
        if ( !m.hasInitialized() )
            return false;

        // console variable auto updater
        foreach ( mCvar *cvarPtr, mt.cvars ) {
            if ( !QString::compare( cvarPtr->name(), ( const char *)args[0] ))
                cvarPtr->update(( const char *)args[1] );
        }
        break;

    case RendererAPI::Shutdown:
        if ( !m.hasInitialized() )
            return false;

        // perform shutdown
        m.shutdown();
        break;

    case RendererAPI::BeginFrame:
        if ( !m.hasInitialized() )
            return false;

        // begin frame
        m.beginFrame();

        // draw platform logo
        cmd.setColour( 1, 1, 1, 1 );
        cmd.drawMaterial( 640/2-256/2, 480/2-256/2, 256, 256, m.platformLogo );

        break;

    case RendererAPI::EndFrame:
        if ( !m.hasInitialized())
            return false;

        // reset colour to null
        cmd.setColour();

        // end frame
        m.endFrame();
        break;

    case RendererAPI::LoadImage:
        if ( !m.hasInitialized())
            return false;

        return ( intptr_t )m.loadImage(( const char *)args[0] );

    case RendererAPI::DrawImage:
        if ( !m.hasInitialized())
            return false;

        cmd.drawImage(( float )args[0], ( float )args[1], ( float )args[2], ( float )args[3],
                      ( float )args[4], ( float )args[5], ( float )args[6], ( float )args[7],
                      ( imgHandle_t )args[8] );
        break;

    case RendererAPI::LoadMaterial:
        if ( !m.hasInitialized())
            return false;

        return ( intptr_t )m.loadMaterial( ( const char *)args[0] );

    case RendererAPI::DrawMaterial:
        if ( !m.hasInitialized())
            return false;

        cmd.drawMaterial(( float )args[0], ( float )args[1], ( float )args[2], ( float )args[3],
                         ( mtrHandle_t )args[4] );
        break;


    case RendererAPI::DrawText:
        if ( !m.hasInitialized())
            return false;

        // save colour, set new font colour
        cmd.saveColour();
        cmd.setColour(( float )args[4], ( float )args[5], ( float )args[6], ( float )args[7], true );

        // draw Qt font
        glImp.drawText(( float )args[0], ( float )args[1], *( QFont* )args[2], ( const char * )args[3] );

        // restore colour
        cmd.restoreColour();

        break;

    case RendererAPI::SetColour:
        if ( !m.hasInitialized())
            return false;

        cmd.setColour(( float )args[0], ( float )args[1], ( float )args[2], ( float )args[3] );
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
    }

    return true;
}
