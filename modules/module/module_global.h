/*
===========================================================================
Copyright (C) 2009-2012 Edd 'Double Dee' Psycho

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

#ifndef MODULE_GLOBAL_H
#define MODULE_GLOBAL_H

//
// includes
//
#include <QtCore/qglobal.h>
#include <QtGui>

// define this if you need renderer calls
//#define RENDERER_ENABLED

//
// defines
//
#if defined( MODULE_BUILD )
#  define MODULESHARED_EXPORT Q_DECL_EXPORT
#else
#  define MODULESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MODULE_GLOBAL_H
