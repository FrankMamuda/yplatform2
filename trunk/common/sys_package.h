/*
===========================================================================
Original software:
===========================================================================
Copyright (c) 1998, Gilles Vollant
All rights reserved.

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
    * The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
    * Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
    * This notice may not be removed or altered from any source distribution.

===========================================================================
C++ rewrite for YPlatform2:
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

#ifndef SYS_PACKAGE_H
#define SYS_PACKAGE_H

//
// includes
//
#include "sys_shared.h"
#include "sys_filesystem.h"
#include "sys_packagefunc.h"
#include <zlib.h>

//
// namespaces
//
namespace Package {
    class Sys_Package;
    static const unsigned int ReadBuffer = 16384;
    static const unsigned int DeflateCompression = 8;
    static const unsigned int OffsetHeaderCompression = 8;
    static const unsigned int OffsetHeaderEntrySize = 18;
    static const unsigned int OffsetHeaderEntryName = 30;
    static const unsigned long IdentHeader = ((0x04<<24)+(0x03<<16)+('K'<<8)+'P');
    static const unsigned long IdentCentralDir = ((0x02<<24)+(0x01<<16)+('K'<<8)+'P');
}

//
// class:Sys_Package
//
class pPackage;
class pEntry;
class Sys_Filesystem;
class Sys_Package : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem package support class" )
    Q_ENUMS( ReadModes )

public:
    // enums
    enum ReadModes {
        Byte    = 1,
        Short   = 2,
        Long    = 4
    };
    pPackage *load( const QString &, int flags );
    template <class T>
    bool readData( fileHandle_t, ReadModes mode, T & );
    pEntry *find( const QString & );

private:
    bool readPackage( pPackage *pPtr );
    QList<pPackage*>packageList;

signals:

public slots:
    void shutdown();
};

//
// externals
//
#ifndef MODULE_BUILD
extern class Sys_Package pkg;
#endif

#endif // SYS_PACKAGE_H
