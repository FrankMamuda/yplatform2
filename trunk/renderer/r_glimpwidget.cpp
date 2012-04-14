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
#include "r_glimpwidget.h"
#include "r_main.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

//
// cvars
//
mCvar *r_hideOnESC;

/*
===============
construct
===============
*/
R_GlimpWidget::R_GlimpWidget( QWidget *parent ) : QGLWidget( parent ) {
    this->setAutoFillBackground( false );
    this->setMouseTracking( true );
}

/*
===============
initializeGL
===============
*/
void R_GlimpWidget::initializeGL() {
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_COLOR_MATERIAL );
    glEnable( GL_POLYGON_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClearDepth( 1.0f );
}

/*
===============
resizeGL
===============
*/
void R_GlimpWidget::resizeGL( int w, int h ) {
    // set 2D virtual screen size
    glViewport( 0, 0, w, h );
    glScissor( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0f, w, h, 0.0f, 0.0f, 1.0f );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

/*
===============
closeEvent
===============
*/
void R_GlimpWidget::closeEvent( QCloseEvent *e ) {
    if ( glImp.hasInitialized())
        gui.raise();

    e->accept();
}

/*
===============
keyPressEvent
===============
*/
void R_GlimpWidget::keyPressEvent( QKeyEvent *e ) {
    // applet updates
    if ( e->isAutoRepeat())
        e->ignore();
    else {
        if ( e->key() == Qt::Key_Escape && r_hideOnESC->integer())
            this->hide();

        mt.keyEvent( ModuleAPI::KeyPress, e->key());
    }
    e->accept();
}

/*
===============
keyReleaseEvent
===============
*/
void R_GlimpWidget::keyReleaseEvent( QKeyEvent *e ) {
    // applet updates
    if ( e->isAutoRepeat())
        e->ignore();
    else
        mt.keyEvent( ModuleAPI::KeyRelease, e->key());

    e->accept();
}

/*
===============
mousePressEvent
===============
*/
void R_GlimpWidget::mousePressEvent( QMouseEvent *e ) {
    // applet updates
    mt.mouseEvent( ModuleAPI::KeyPress, e->button());
    e->accept();
}

/*
===============
mouseReleaseEvent
===============
*/
void R_GlimpWidget::mouseReleaseEvent( QMouseEvent *e ) {
    // applet updates
    mt.mouseEvent( ModuleAPI::KeyRelease, e->button());
    e->accept();
}

/*
===============
mouseDoubleClickEvent
===============
*/
void R_GlimpWidget::mouseDoubleClickEvent( QMouseEvent *e ) {
    // applet updates
    mt.mouseEvent( ModuleAPI::DoubleClick, e->button());
    e->accept();
}

/*
===============
mouseMoveEvent
===============
*/
void R_GlimpWidget::mouseMoveEvent( QMouseEvent *e ) {
    float x, y, h, v;

    // get scale factors
    h = Renderer::HorizontalScreenModes[glImp.getScreenMode()] / static_cast<float>( Renderer::HorizontalScreenModes[Renderer::DefaultScreenMode] );
    v = Renderer::VerticalScreenModes[glImp.getScreenMode()]   / static_cast<float>( Renderer::VerticalScreenModes[Renderer::DefaultScreenMode] );

    // get pos and adjust it
    x = e->x() / h;
    y = e->y() / v;

    // applet updates
    mt.mouseMotion( static_cast<int>( x ), static_cast<int>( y ));
    e->accept();
}

/*
===============
wheelEvent
===============
*/
void R_GlimpWidget::wheelEvent( QWheelEvent *e ) {
    mt.wheelEvent( e->delta());
    e->accept();
}
