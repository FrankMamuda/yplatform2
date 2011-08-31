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

//
// commands
//
createCommand( m, listImages )
createCommand( m, listMaterials )

/*
===================
init
===================
*/
void R_Main::init() {
    int y;

    // init cvars
    r_screenMode = mt.cvarCreate( "r_screenMode", QString( "%1" ).arg( Renderer::DefaultScreenMode ), pCvar::Archive, "renderer screen dimensions" );
    r_adjustScreen = mt.cvarCreate( "r_adjustScreen", "1", pCvar::Archive, "adjust screen to virtual coordinates" );

    // create screen
    glImp.init();

    // add supported extensions
    this->extensionList << ".jpg" << ".tga" << ".png";

    // load default image
    m.defaultImage = this->loadImage( Renderer::DefaultImage );

    // add commands
    mt.cmdAdd( "r_listImages", listImagesCmd, this->tr( "list loaded images" ));
    mt.cmdAdd( "r_listMaterials", listMaterialsCmd, this->tr( "list loaded materials" ));

    // load materials
    mLib.init();

    // load platform logo
    m.platformLogo = this->loadMaterial( Renderer::PlatformLogo );

    // init function tables
    for ( y = 0; y < Renderer::FuncTableSize; y++ ) {
        this->funcTable[GenFunc::None][y] = 0.0f;
        this->funcTable[GenFunc::Sine][y] = sin( degreesToRadians( y * 360.0f / (( float )( Renderer::FuncTableSize - 1 ))));
        this->funcTable[GenFunc::Square][y] = ( y < Renderer::FuncTableSize / 2 ) ? 1.0f : -1.0f;
        this->funcTable[GenFunc::SawTooth][y] = ( float )y / Renderer::FuncTableSize;
        this->funcTable[GenFunc::InverseSawtooth][y] = 1.0f - this->funcTable[GenFunc::SawTooth][y];

        if ( y < Renderer::FuncTableSize / 2 ) {
            if ( y < Renderer::FuncTableSize / 4 )
                this->funcTable[GenFunc::Triangle][y] = ( float )y / ( Renderer::FuncTableSize / 4 );
            else
                this->funcTable[GenFunc::Triangle][y] = 1.0f - this->funcTable[GenFunc::Triangle][y - Renderer::FuncTableSize / 4];
        } else
            this->funcTable[GenFunc::Triangle][y] = -this->funcTable[GenFunc::Triangle][y - Renderer::FuncTableSize / 2];
    }

    // add settings widget
    this->settingsWidget = new R_Settings();
    mt.guiAddSettingsTab( this->settingsWidget, "Renderer" /*, ":/common/platform"*/ );
    //this->connect( r_screenMode, SIGNAL( valueChanged( QString, QString )), this->settingsWidget, SLOT( intializeCvars()));
    //this->connect( r_adjustScreen, SIGNAL( valueChanged( QString, QString )), this->settingsWidget, SLOT( intializeCvars()));

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
    mt.comPrint( this->tr( "Image list:\n" ));
    foreach ( R_Image *img, this->imageList ) {
        mt.comPrint( this->tr( " %1: w %2 h %3 tex %4\n" ).arg( img->name()).arg( img->width()).arg( img->height()).arg(( unsigned int )img->texture ));
    }
}

/*
===================
listMaterials
===================
*/
void R_Main::listMaterials() {
    mt.comPrint( this->tr( "Material list:\n" ));
    foreach ( R_Material *mtr, this->mtrList ) {
        mt.comPrint( this->tr( " %1: stages %2\n" ).arg( mtr->name()).arg( mtr->stageList.count()));
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
        mt.comError( Sys_Common::SoftError, this->tr( "R_Main::loadImage: called with empty name\n" ));
        return -1;
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
            delete imgPtr;
            imgPtr = new R_Image( QFileInfo( filename ).path() + "/" + QFileInfo( filename ).baseName() + ext, mode, this );
            if ( imgPtr->isValid())
                break;
        }
    }

    // giving up, set default image
    if ( !imgPtr->isValid()) {
        // did not find a valid texture, revert to default
        mt.comPrint( this->tr( "^3WARNING: R_Main::loadImage: could not find image \'%1\', setting default\n" ).arg( filename ));
        delete imgPtr;

        // ..if we have a default (should not happen since default img is in internal assets)
        if ( !QString::compare( filename, Renderer::DefaultImage )) {
            mt.comError( Sys_Common::FatalError, this->tr( "R_Main::loadImage: cannot load default image\n" ));
            m.shutdown();
            return -1;
        }

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
        mt.comError( Sys_Common::SoftError, this->tr( "R_Main::loadMaterial: called with empty name\n" ));
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
    this->setTime( mt.comMilliseconds() * 0.001f );

    // update mtr scripts
    mLib.update();
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
void R_Main::shutdown() {
    // stop evaluating material libraries
    mLib.shutdown();

    // clear images
    foreach ( R_Image *imgPtr, this->imageList )
        delete imgPtr;
    this->imageList.clear();

    // must clear all stages
    // not just the ones in the material
    // since scripting allows to define stage
    // as a separate entity
    foreach ( R_MaterialStage *mtrStagePtr, this->mtrStageList )
        delete mtrStagePtr;
    this->mtrStageList.clear();

    // clear materials
    foreach ( R_Material *mtrPtr, this->mtrList )
        delete mtrPtr;
    this->mtrList.clear();

    // destroy screen
    glImp.shutdown();

    // remove commands
    mt.cmdRemove( "r_listImages" );
    mt.cmdRemove( "r_listMaterials" );

    // we are down
    this->setInitialized( false );
}
