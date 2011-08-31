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

#ifndef R_GLIMPWIDGET_H
#define R_GLIMPWIDGET_H

//
// includes
//
#include "r_shared.h"

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
}

//
// class:R_GlimpWidget
//
class R_GlimpWidget : public QGLWidget {
    Q_OBJECT
    Q_CLASSINFO( "description", "Renderer window" )

protected:
    void resizeGL( int w, int h );

public:
    R_GlimpWidget( QWidget *parent );
};

#endif // R_GLIMPWIDGET_H
