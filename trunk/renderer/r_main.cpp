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
#include "r_main.h"
#include "r_glimp.h"
#include "r_mtrlib.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"
#include "../common/sys_cmd.h"

//
// classes
//
class R_Main m;

//
// cvars
//
extern mCvar *r_screenMode;
extern mCvar *r_adjustScreen;
extern mCvar *r_hideOnESC;

//
// commands
//
createSimpleCommand( m, listImages )
createSimpleCommand( m, listMaterials )

/*
===================
init
===================
*/
void R_Main::init( bool reload ) {
    int y;

    if ( !reload ) {
        // init cvars
        r_screenMode = cv.create( "r_screenMode", QString( "%1" ).arg( Renderer::DefaultScreenMode ), pCvar::Archive, "renderer screen dimensions" );
        r_adjustScreen = cv.create( "r_adjustScreen", "1", pCvar::Archive, "adjust screen to virtual coordinates" );
        r_hideOnESC = cv.create( "r_hideOnESC", "1", pCvar::NoFlags, "hide renderer upon pressing ESC key" );

        // create screen
        glImp.init();

        // add supported extensions
        this->extensionList << ".jpg" << ".tga" << ".png";

        // add commands
        cmd.add( "r_listImages", listImagesCmd, this->tr( "list loaded images" ));
        cmd.add( "r_listMaterials", listMaterialsCmd, this->tr( "list loaded materials" ));

        // init function tables
        for ( y = 0; y < Renderer::FuncTableSize; y++ ) {
            this->funcTable[GenFunc::None][y] = 0.0f;
            this->funcTable[GenFunc::Sine][y] = sin( degreesToRadians( y * 360.0f / ( static_cast<float>( Renderer::FuncTableSize - 1 ))));
            this->funcTable[GenFunc::Square][y] = ( y < Renderer::FuncTableSize / 2 ) ? 1.0f : -1.0f;
            this->funcTable[GenFunc::SawTooth][y] = static_cast<float>( y ) / Renderer::FuncTableSize;
            this->funcTable[GenFunc::InverseSawtooth][y] = 1.0f - this->funcTable[GenFunc::SawTooth][y];

            if ( y < Renderer::FuncTableSize / 2 ) {
                if ( y < Renderer::FuncTableSize / 4 )
                    this->funcTable[GenFunc::Triangle][y] = static_cast<float>( y ) / ( Renderer::FuncTableSize / 4 );
                else
                    this->funcTable[GenFunc::Triangle][y] = 1.0f - this->funcTable[GenFunc::Triangle][y - Renderer::FuncTableSize / 4];
            } else
                this->funcTable[GenFunc::Triangle][y] = -this->funcTable[GenFunc::Triangle][y - Renderer::FuncTableSize / 2];
        }

        // add settings widget
        this->settingsWidget = new R_Settings();
        gui.addSettingsTab( this->settingsWidget, "Renderer" /*, ":/common/platform"*/ );

    }

    // load default image
    m.defaultImage = this->loadImage( Renderer::DefaultImage );

    // load materials
    mLib.init();

    // load platform logo
    m.platformLogo = this->loadMaterial( Renderer::PlatformLogo );

    // all done
    this->setInitialized();
}

/*
===================
degreesToRadians
===================
*/
double R_Main::degreesToRadians( double degrees ) {
    return degrees *= ( M_PI / 180.0f );
}

/*
===================
listImages
===================
*/
void R_Main::listImages() {
    com.print( Sys::ColourYellow + this->tr( "Image list:\n" ));
    foreach ( R_Image *img, this->imageList ) {
        com.print( this->tr( " %1: w %2 h %3 tex %4\n" ).arg( img->name()).arg( img->width()).arg( img->height()).arg(( unsigned int )img->texture ));
    }
}

/*
===================
listMaterials
===================
*/
void R_Main::listMaterials() {
    com.print( Sys::ColourYellow + this->tr( "Material list:\n" ));
    foreach ( R_Material *mtr, this->mtrList ) {
        com.print( this->tr( " %1: stages %2\n" ).arg( mtr->name()).arg( mtr->stageList.count()));
    }
}

/*
===================
loadImage
===================
*/
imgHandle_t R_Main::loadImage( const QString &filename, R_Image::ClampModes mode ) {
    int y = 0;

    // abort on empty name
    if ( filename.isEmpty()) {
        com.error( StrSoftError + this->tr( "called with empty name\n" ));
        return -1;
    }

    // check if we have already marked it as missing
    foreach ( QString missing, this->missingList ) {
        if ( !QString::compare( filename, missing ))
            return m.defaultImage;
    }

    // check if it exists
    foreach ( R_Image *imgPtr, this->imageList ) {
        if ( !QString::compare( imgPtr->name(), QString( QFileInfo( filename ).path() + "/" + QFileInfo( filename ).baseName()))) {
            // might be the same image, but clamp modes differ
            if ( imgPtr->clampMode() == mode )
                return y;
        }
        y++;
    }

    // load directly
    R_Image *imgPtr = new R_Image( filename, mode, this );

    // this may happen if we have no extension or file simply does not exist
    if ( !imgPtr->isValid()) {
        foreach ( QString ext, this->extensionList ) {
            imgPtr->reload( QFileInfo( filename ).path() + "/" + QFileInfo( filename ).baseName() + ext, mode );
            if ( imgPtr->isValid())
                break;
        }
    }

    // giving up, set default image
    if ( !imgPtr->isValid()) {
        // did not find a valid texture, revert to default
        com.print( StrWarn + this->tr( "could not find image \'%1\', setting default\n" ).arg( filename ));
        delete imgPtr;

        // ..if we have a default (should not happen since default img is in internal assets)
        if ( !QString::compare( filename, Renderer::DefaultImage )) {
            com.error( StrFatalError + this->tr( "cannot load default image\n" ));
            m.shutdown();
            return -1;
        }
        this->missingList << filename;

        // return default image
        return m.defaultImage;
    }

    this->imageList << imgPtr;
    return this->imageList.count()-1;
}

/*
===================
loadMaterial
===================
*/
mtrHandle_t R_Main::loadMaterial( const QString &mtrName ) {
    int y = 0;

    // abort on empty name
    if ( mtrName.isEmpty()) {
        com.error( StrSoftError + this->tr( "called with empty name\n" ));
        return -1;
    }

    // check if it exists
    foreach ( R_Material *mtrPtr, this->mtrList ) {
        if ( !QString::compare( mtrPtr->name(), mtrName ))
            return y;
        y++;
    }

    R_Material *mtrPtr = new R_Material( mtrName, this );
    this->mtrList << mtrPtr;
    return this->mtrList.count()-1;
}

/*
===================
beginFrame
===================
*/
void R_Main::beginFrame() {
    // clear screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();

    // set time
    this->setTime( com.milliseconds() * 0.001f );

    // update mtr scripts
    mLib.mse->update();
}

/*
===================
endFrame
===================
*/
void R_Main::endFrame() {
    glFlush();

    // update screen
    glImp.update();
}

/*
===================
shutdown
===================
*/
void R_Main::shutdown( bool reload ) {
    // stop evaluating material libraries
    mLib.shutdown();

    // clear images, materials and stages
    // assuming script takes care of its own garbage
    this->imageList.clear();
    this->mtrStageList.clear();
    this->mtrList.clear();

    if ( !reload ) {
        // destroy screen
        glImp.shutdown();

        // remove commands
        cmd.remove( "r_listImages" );
        cmd.remove( "r_listMaterials" );

        // remove settings widget
        gui.removeSettingsTab( "Renderer" );
    }

    // we are down
    this->setInitialized( false );
}
