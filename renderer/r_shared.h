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

#ifndef R_SHARED_H
#define R_SHARED_H

//
// defines
//
typedef int imgHandle_t;
typedef int mtrHandle_t;
typedef int fontHandle_t;

//
// includes
//
#ifdef R_BUILD
#include <QtGui>
#include <QtOpenGL>
#ifdef Q_OS_WIN
#include <windows.h>
#include <gl/gl.h>
#endif
#include "../common/sys_shared.h"

//
// namespaces
//
namespace Renderer {
    static const QString DefaultImage ( ":/common/default" );
    static const QString PlatformLogo ( ":/common/platform" );
    static const QString Title        ( QObject::trUtf8( "Nu Renderer" ));
    static const QString Copyright    ( QObject::trUtf8( "Copyright (c) 2011, Edd 'Double Dee' Psycho." ));
    static const QString Version      ( QObject::trUtf8( "v0.98.43 (beta)" ));
    typedef float Vec2D[2];
    typedef float Vec4D[4];
}

// fast float to int conversion
#if id386 && !defined ( __GNUC__ )
long myftol( float f );
#else
#define myftol( x )(( int )( x ))
#endif

// define clamp to edge
#if !defined( GL_CLAMP_TO_EDGE )
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#endif // R_BUILD
#endif // R_SHARED_H
