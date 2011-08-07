/*
===========================================================================
Copyright (C) 2009-2011 Edd 'Double Dee' Psycho

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

#ifndef R_PUBLIC_H
#define R_PUBLIC_H

//
// includes
//
#include "../modules/mod_public.h"
#include "r_shared.h"
#include "r_font.h"

// current api version
namespace RendererAPI {
    static const unsigned int Version = 1;

    // public API calls (renderer)
    enum RendererAPICalls {
        LoadImage = 0,
        DrawImage,
        LoadMaterial,
        DrawMaterial,
        LoadFont,
        SetColour
    };
}

//
// exports
//
typedef struct rendererExport_s {
    // init and shutdown
    void ( *init )();
    void ( *shutdown )();
    bool ( *initialized )();

    // object loading
    imgHandle_t ( *loadImage )( const QString & );
    mtrHandle_t ( *loadMaterial )( const QString & );
    bool ( *loadFont )( const QString &, int, fontInfo_t & );

    // renderering
    void ( *setColour )( const QColor & );
    void ( *drawImage )( float, float, float, float, float, float, float, float, imgHandle_t );
    void ( *drawMaterial )( float, float, float, float, mtrHandle_t );
    void ( *beginFrame )();
    void ( *endFrame )();

    // window specific
    void ( *raise )();
    void ( *hide )();

    // common
    void ( *updateCvar )( const QString &, const QString & );
} rendererExport_t;

#endif // R_PUBLIC_H
