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
#include "r_renderer.h"
#include "r_cmd.h"
#include "r_main.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

//
// cvars
//
mCvar *r_hideOnESC;
mCvar *r_screenMode;
mCvar *r_adjustScreen;

//
// FIXME: adjust is BROKEN
//

/*
===============
construct
===============
*/
R_Renderer::R_Renderer( QWindow *parent ): QWindow( parent ), m_context( 0 ), m_device( 0 ) {
    this->setSurfaceType( QWindow::OpenGLSurface );
}

/*
===============
makeContext
===============
*/
void R_Renderer::makeContext() {
    if ( this->m_context == NULL ) {
        this->m_context = new QOpenGLContext(this);
        this->m_context->setFormat( this->requestedFormat());
        this->m_context->create();
    }

    this->m_context->makeCurrent( this );
    this->initializeOpenGLFunctions();
    this->initialize();
}

/*
===============
makeContext
===============
*/
void R_Renderer::initialize() {
    glMatrixMode( GL_PROJECTION ); // signal that I want to work with the projection stack
    glLoadIdentity(); // make sure that the projection stack doesn't already have anything on it
    glMatrixMode( GL_MODELVIEW ); // the rest of my app will only change MODELVIEW
    glLoadIdentity(); // make sure that the modelview stack doesn't already have anything on it

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

/*
===============
render
===============
*/
void R_Renderer::renderText( int x, int y, const QString &text, const QFont &font ) {
    if ( text.isEmpty())
        return;

    if ( this->m_painter == NULL )
        return;

    this->m_painter->setPen( rCmd.colour );
    this->m_painter->setFont( font );
    this->m_painter->drawText( x, y, text );
    this->m_painter->end();
}

/*
===============
render
===============
*/
void R_Renderer::render() {
    if ( this->isExposed())
        return;

    glClearColor( 0.f, 0.f, 0.f, 1.f );
    glClear( GL_COLOR_BUFFER_BIT );
    glViewport( 0, 0, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glScissor( 0, 0, this->width(), this->height());
    glOrtho( 0.0f, this->width(), this->height(), 0.0f, 0.0f, 1.0f );
    rCmd.setBlendMode( R_Cmd::SrcBlend::SrcAlpha, R_Cmd::DstBlend::MinusSrcAlpha );

    // draw platform logo
    rCmd.setColour( Renderer::ColourWhite );
    if ( r_adjustScreen->integer())
        rCmd.drawMaterial( Renderer::HorizontalScreenModes[Renderer::DefaultScreenMode]/2-256/2,
                Renderer::VerticalScreenModes[Renderer::DefaultScreenMode]/2-256/2,
                256, 256, m.platformLogo );
    else
        rCmd.drawMaterial( Renderer::HorizontalScreenModes[this->getScreenMode()]/2-256/2,
                Renderer::VerticalScreenModes[this->getScreenMode()]/2-256/2,
                256, 256, m.platformLogo );

    QFont font;
    font.setPointSize( 48 );

    if ( this->m_device == NULL )
        this->m_device = new QOpenGLPaintDevice;

    this->m_device->setSize( this->size());
    this->m_painter = new QPainter( this->m_device );

    // NOTE: this doesn't work in opengl es, revert to bitmap text rendering?
    rCmd.setColour( 1, 0, 0, 1 );

    this->renderText( 43, 42, "AAAA", font );

    delete this->m_painter;
    this->m_painter = NULL;
}

/*
===============
begin
===============
*/
void R_Renderer::begin() {
    if ( this->m_context == NULL )
        return;

    this->m_context->makeCurrent( this );
    m.beginFrame();
    this->render();
}

/*
===============
resizeEvent
===============
*/
void R_Renderer::resizeEvent( QResizeEvent *ePtr ) {


    this->initialize();
    glViewport( 0, 0, width() * this->devicePixelRatio(), height() * this->devicePixelRatio());
    glScissor( 0, 0, this->width(), this->height());
    glOrtho( 0.0f, this->width(), this->height(), 0.0f, 0.0f, 1.0f );
    this->m_context->makeCurrent( this );

    //glFlush();

    /*
    Q_D(QOpenGLWidget);
    d->w = width();
    d->h = height();
    d->initialize();


    d->context.makeCurrent(d->surface());
    delete d->fbo; // recreate when resized
    d->fbo = new QOpenGLFramebufferObject(size() * devicePixelRatio(), QOpenGLFramebufferObject::CombinedDepthStencil);
    d->fbo->bind();
    QOpenGLFunctions *funcs = d->context.functions();
    funcs->glBindTexture(GL_TEXTURE_2D, d->fbo->texture());
    funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    resizeGL(width(), height());
    paintGL();
    funcs->glFlush();*/
}

/*
===============
destruct
===============
*/
R_Renderer::~R_Renderer() {
    delete this->m_device;
}

/*
===============
end
===============
*/
void R_Renderer::end() {
    if ( this->isExposed()) {
        glFlush();
        this->m_context->swapBuffers( this );
    }

    m.endFrame();
}

/*
===============
keyPressEvent
===============
*/
void R_Renderer::keyPressEvent( QKeyEvent *e ) {
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
void R_Renderer::keyReleaseEvent( QKeyEvent *e ) {
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
void R_Renderer::mousePressEvent( QMouseEvent *e ) {
    // applet updates
    mt.mouseEvent( ModuleAPI::KeyPress, e->button());
    e->accept();
}

/*
===============
mouseReleaseEvent
===============
*/
void R_Renderer::mouseReleaseEvent( QMouseEvent *e ) {
    // applet updates
    mt.mouseEvent( ModuleAPI::KeyRelease, e->button());
    e->accept();
}

/*
===============
mouseDoubleClickEvent
===============
*/
void R_Renderer::mouseDoubleClickEvent( QMouseEvent *e ) {
    // applet updates
    mt.mouseEvent( ModuleAPI::DoubleClick, e->button());
    e->accept();
}


/*
===============
getScreenMode
===============
*/
int R_Renderer::getScreenMode() {
    int currentMode = r_screenMode->integer();

    if ( currentMode >= 0 && currentMode < Renderer::NumScreenModes )
        return currentMode;
    else {
        com.error( StrSoftError + this->tr( "invalid screen mode '%1', setting default\n" ));
        r_screenMode->set( Renderer::DefaultScreenMode );
        return Renderer::DefaultScreenMode;
    }
}

/*
===============
adjustCoords

 disallow screen modes, just use aspect ratio??? and zoom modes
===============
*/
void R_Renderer::adjustCoords( float &x, float &y ) {
    // do nothing for now
    /*if ( r_adjustScreen->integer()) {
        // adjust input values
        x *= this->horizontalFactor();
        y *= this->verticalFactor();
    }*/
}


/*
===============
mouseMoveEvent
===============
*/
void R_Renderer::mouseMoveEvent( QMouseEvent *e ) {
    float x, y, h, v;

    // get scale factors
    h = Renderer::HorizontalScreenModes[this->getScreenMode()] / static_cast<float>( Renderer::HorizontalScreenModes[Renderer::DefaultScreenMode] );
    v = Renderer::VerticalScreenModes[this->getScreenMode()]   / static_cast<float>( Renderer::VerticalScreenModes[Renderer::DefaultScreenMode] );

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
void R_Renderer::wheelEvent( QWheelEvent *e ) {
    mt.wheelEvent( e->delta());
    e->accept();
}

/*
===============
closeEvent
===============
*/
void R_Renderer::closeEvent( QCloseEvent *e ) {
    gui.raise();
    e->accept();
}
