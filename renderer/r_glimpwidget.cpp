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
#include "r_glimpwidget.h"
#include "r_main.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

/*
===============
construct
===============
*/
R_GlimpWidget::R_GlimpWidget( QWidget *parent ) : QGLWidget( parent ) {
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
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
    glOrtho( 0, w, h, 0, 0, 1 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}
