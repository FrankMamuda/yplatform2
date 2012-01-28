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

#ifndef MOD_MAIN_H
#define MOD_MAIN_H

//
// includes
//
#include "module_global.h"

//
// class::Mod_Main
//
class MODULESHARED_EXPORT Mod_Main : public QObject {
    Q_OBJECT

public:
    Mod_Main();
};

#endif // MOD_MAIN_H
