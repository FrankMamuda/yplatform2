/*
===========================================================================
Copyright (C) 2011-2016 Edd 'Double Dee' Psycho

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

#ifndef R_RENDERER_H
#define R_RENDERER_H

//
// includes
//
#include "r_shared.h"
#include <QWindow>
#include <QOpenGLFunctions>

//
// namespaces
//
namespace Renderer {
    static const Vec4D ColourWhite   = { 1, 1, 1, 1 };
    static const Vec4D ColourBlack   = { 0, 0, 0, 1 };
    static const Vec4D ColourRed     = { 1, 0, 0, 1 };
    static const Vec4D ColourGreen   = { 0, 1, 0, 1 };
    static const Vec4D ColourBlue    = { 0, 0, 1, 1 };
    static const Vec4D ColourMagenta = { 1, 0, 1, 1 };
    static const Vec4D ColourCyan    = { 0, 1, 1, 1 };
    static const Vec4D ColourYellow  = { 1, 1, 0, 1 };
    const static int NumScreenModes = 6;
    const static int DefaultScreenMode = 1;
    const static int MaximumTextureSize = 4096;
    const static int HorizontalScreenModes[NumScreenModes] = { 320, 640, 800, 1024, 1280, 1600 };
    const static int VerticalScreenModes[NumScreenModes]   = { 240, 480, 600,  768, 1024, 1200 };
}

//
// classs: R_Renderer
//
class R_Renderer : public QWindow, protected QOpenGLFunctions {
    Q_CLASSINFO( "description", "OpenGL Implementation" )
    Q_OBJECT
    Q_PROPERTY( float horizontalFactor READ horizontalFactor WRITE setHorizontalFactor )
    Q_PROPERTY( float verticalFactor READ verticalFactor WRITE setVerticalFactor )

public:
    R_Renderer( QWindow * = 0 );
    ~R_Renderer();
    void initialize(); Q_DECL_OVERRIDE
    void render() Q_DECL_OVERRIDE;
    void adjustCoords( float &x, float &y );
    int getScreenMode();
    float horizontalFactor() const { return this->m_horizontalFactor; }
    float verticalFactor() const { return this->m_verticalFactor; }

protected:
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void wheelEvent( QWheelEvent *);
    virtual void closeEvent( QCloseEvent *e );
    virtual void resizeEvent( QResizeEvent * );

public slots:
    void setHorizontalFactor( float value = 1.0f ) { this->m_horizontalFactor = value; }
    void setVerticalFactor( float value = 1.0f ) { this->m_verticalFactor = value; }
    void makeContext();
    void begin();
    void end();
    void renderText( int x, int y, const QString &text, const QFont &font );

private:
    float m_horizontalFactor;
    float m_verticalFactor;
    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
    QPainter *m_painter;
};

#endif // R_GLIMPWIDGET_H
