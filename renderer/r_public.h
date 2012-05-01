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

#ifndef R_PUBLIC_H
#define R_PUBLIC_H

//
// includes
//
#include "../modules/mod_public.h"
#include "r_shared.h"

// fixes drawtext issue
#if defined( DrawText )
#undef DrawText
#endif

// current api version
namespace RendererAPI {
    static const unsigned int Version = 5;

    enum WindowState {
        Raised = 0,
        Hidden
    };

    // public API calls (renderer)
    enum RendererAPICalls {
        ModAPI = 0,
        Init,
        UpdateCvar,
        Shutdown,
        BeginFrame,
        EndFrame,
        LoadMaterial,
        DrawMaterial,
        DrawText,
        SetColour,
        Raise,
        Hide,
        State,
        Reload,
        SetWindowTitle
    };
}

#endif // R_PUBLIC_H
