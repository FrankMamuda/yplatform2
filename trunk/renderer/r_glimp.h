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

#ifndef R_GLIMP_H
#define R_GLIMP_H

//
// includes
//
#include "r_shared.h"
#include "r_glimpwidget.h"

//
// namespaces
//
namespace Renderer {
    const static int NumScreenModes = 6;
    const static int DefaultScreenMode = 1;
    const static int MaximumTextureSize = 4096;
    const static int HorizontalScreenModes[NumScreenModes] = { 320, 640, 800, 1024, 1280, 1600 };
    const static int VerticalScreenModes[NumScreenModes]   = { 240, 480, 600,  768, 1024, 1200 };
};

//
// class:R_GLimp
//
class R_GLimp : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "OpenGL Implementation" )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_PROPERTY( float horizontalFactor READ horizontalFactor WRITE setHorizontalFactor )
    Q_PROPERTY( float verticalFactor READ verticalFactor WRITE setVerticalFactor )

public:
    void init();
    void shutdown();
    void adjustCoords( float &x, float &y );
    int getScreenMode();

    // property getters
    bool hasInitialized() const { return this->m_initialized; }
    float horizontalFactor() const { return this->m_horizontalFactor; }
    float verticalFactor() const { return this->m_verticalFactor; }

    // renderer window
    R_GlimpWidget *widget;
    int glVersionFlags;

private:
    // properties
    bool m_initialized;
    float m_horizontalFactor;
    float m_verticalFactor;

public slots:
    void resizeScreen();
    void update();
    void drawText( float x, float y, QFont font, const QString &text );
    void setWindowTitle( const QString &title );

    // property setters
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void setHorizontalFactor( float value = 1.0f ) { this->m_horizontalFactor = value; }
    void setVerticalFactor( float value = 1.0f ) { this->m_verticalFactor = value; }
};

//
// externals
//
#ifdef R_BUILD
extern class R_GLimp glImp;
#endif

#endif // R_GLIMP_H
