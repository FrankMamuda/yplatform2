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
#include "r_glimp.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"
#include "../common/sys_cmd.h"

//
// classes
//
R_GLimp glImp;

//
// cvars
//
mCvar *r_screenMode;
mCvar *r_adjustScreen;

//
// commands
//
createCommandPtr( glImp.widget, hide );
createCommandPtr( glImp.widget, show );

/*
===============
init
===============
*/
void R_GLimp::init() {
    mt.comPrint( this->tr( "^2R_GLimp::init: ^5initializing OpenGL display\n" ));
    QGLFormat format;

#ifndef Q_OS_WIN
    // breaks compatibility with intel inegrated graphics on winxp
    if (( format.openGLVersionFlags() & format.OpenGL_Version_1_5 ) == 0 )
        mt.comError( Sys_Common::FatalError, this->tr( "R_GLimp::init: OpenGL version 1.5 or higher is required\n" ));
#endif

    // set format
    format.setSampleBuffers( true );
    format.setDoubleBuffer( true );
    format.setDirectRendering( true );

    // create context
    this->context = new QGLContext( format );

    // create openGL widget
    this->widget = new R_GlimpWidget( this->widget );
    this->widget->setFormat( format );
    this->widget->setAutoFillBackground( false );
    this->widget->setMouseTracking( true );
    this->widget->makeCurrent();

    // set default paint engine as OpenGL
    QGL::setPreferredPaintEngine( QPaintEngine::OpenGL );

    // init widget
    this->widget->setWindowTitle( Renderer::Title );
    this->widget->setWindowIcon( QIcon( Renderer::PlatformLogo ));
    this->widget->show();

    // update screen on cvar changes
    this->connect( r_screenMode, SIGNAL( valueChanged( QString, QString )), this, SLOT(resizeScreen()));
    this->resizeScreen();

    // add cmds
    mt.cmdAdd( "r_raise", showCmd, this->tr( "raise renderer window" ));
    mt.cmdAdd( "r_hide", hideCmd, this->tr( "hide renderer window" ));

    // glimp is ok
    this->setInitialized();
}

/*
===============
resizeScreen
===============
*/
void R_GLimp::resizeScreen() {
    float h, v;

    // get scale factors
    h = Renderer::HorizontalScreenModes[this->getScreenMode()] / (float)Renderer::HorizontalScreenModes[Renderer::DefaultScreenMode];
    v = Renderer::VerticalScreenModes[this->getScreenMode()]   / (float)Renderer::VerticalScreenModes[Renderer::DefaultScreenMode];

    // check for abnormal sizes
    if ( h == 0.0f || v == 0.0f || h == 10.0f || v == 10.0f ) {
        h = 1.0f;
        v = 1.0f;
    }
    this->setHorizontalFactor( h );
    this->setVerticalFactor( v );

    // set size
    this->widget->setFixedSize( Renderer::HorizontalScreenModes[this->getScreenMode()], Renderer::VerticalScreenModes[this->getScreenMode()] );
}

/*
===============
update
===============
*/
void R_GLimp::update() {
    if ( this->hasInitialized())
        this->widget->update();
}

/*
===============
shutdown
===============
*/
void R_GLimp::shutdown() {
    if ( this->hasInitialized()) {
        this->widget->close();

        // announce
        mt.comPrint( this->tr( "^3R_GLimp: shutdown\n" ));

        // remove cmds
        mt.cmdRemove( "r_raise" );
        mt.cmdRemove( "r_hide" );

        // clean up
        delete this->context;
        this->widget->~QGLWidget();
        this->setInitialized( false );
    }
}

/*
===============
drawText
===============
*/
void R_GLimp::drawText( float x, float y, QFont font, const QString &text ) {
    int pixelSize;
    this->adjustCoords( x, y );

    // adjust pointSize
    pixelSize = font.pixelSize();
    if ( pixelSize != pixelSize * this->horizontalFactor())
        font.setPixelSize( pixelSize * this->horizontalFactor());

    // render font
    this->widget->renderText( x, y, 0.0f, text, font );
}

/*
===============
getScreenMode
===============
*/
int R_GLimp::getScreenMode() {
    int currentMode = r_screenMode->integer();

    if ( currentMode >= 0 && currentMode < Renderer::NumScreenModes )
        return currentMode;
    else {
        mt.comError( Sys_Common::SoftError, this->tr( "R_GLimp::getScreenMode: invalid screen mode '%1', setting default\n" ));
        r_screenMode->set( Renderer::DefaultScreenMode );
        return Renderer::DefaultScreenMode;
    }
}

/*
===============
adjustCoords
===============
*/
void R_GLimp::adjustCoords( float &x, float &y ) {
    if ( r_adjustScreen->integer()) {
        // adjust input values
        x *= this->horizontalFactor();
        y *= this->verticalFactor();
    }
}
