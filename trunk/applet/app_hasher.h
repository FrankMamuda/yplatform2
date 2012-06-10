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

#ifndef APP_HASHER_H
#define APP_HASHER_H

//
// includes
//
#include "../common/sys_shared.h"

//
// class:App_Hasher
//
class App_Hasher : public QObject {
    Q_OBJECT

public:
    QString encrypt( const QString & );
};

//
// externals
//
#ifndef MODULE_BUILD
extern App_Hasher hash;
#endif

#endif // APP_HASHER_H
