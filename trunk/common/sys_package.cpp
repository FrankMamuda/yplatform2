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

//
// complete rewrite of package handling
// zip is one weird file format indeed
// TODO: add gzip support?
//

//
// includes
//
#include "sys_common.h"
#include "sys_filesystem.h"
#include "sys_package.h"
#include "sys_packagefunc.h"

//
// classes
//
class Sys_Package pkg;

/*
============
readData
============
*/
template <class T>
bool Sys_Package::readData( fileHandle_t handle, ReadModes mode, T &out ) {
    T data = 0;
    byte buffer;
    int y;

    // perform read in desired mode
    // mind endianess
    for ( y = 0; y < mode; y++ ) {
        if ( fs.read( &buffer, 1, handle, Sys_Filesystem::Silent ))
            data += ( static_cast<unsigned int>( buffer )) << 8 * y;
        else {
            out = 0;
            return false;
        }
    }
    out = data;
    return true;
}

/*
============
readPackage
============
*/
bool Sys_Package::readPackage( pPackage *pPtr ) {
    unsigned long tmp, y = 0;
    int c, k, entryNameSize, skip;
    QString eName;

    // enter parse loop
    while ( 1 ) {
        // read ident
        if ( this->readData( pPtr->fileHandle(), Long, tmp )) {
            if ( tmp == Package::IdentHeader ) {
                // alloc a new file
                pEntry *pEntryPtr = new pEntry();

                // seek until compression method
                if ( !fs.seek( pPtr->fileHandle(), y + Package::OffsetHeaderCompression, Sys_Filesystem::Silent ))
                    goto corruptHeader;

                // read compression method
                if ( this->readData( pPtr->fileHandle(), Short, tmp )) {
                    if ( tmp == 0 ) {
                        // set mode for file
                        pEntryPtr->setCompressionMode( pEntry::NoCompression );
                    } else if ( tmp == Package::DeflateCompression ) {
                        // set mode for file
                        pEntryPtr->setCompressionMode( pEntry::Compressed );
                    } else {
                        delete pEntryPtr;
                        com.error( Sys_Common::SoftError, this->tr( "Sys_Package::readPackage: unknown compression mode '%1' in \"%2\" ident\n" ).arg( tmp ).arg( pPtr->name()));
                        break;
                    }
                }

                // seek until entry size
                if ( !fs.seek( pPtr->fileHandle(), y + Package::OffsetHeaderEntrySize, Sys_Filesystem::Silent ))
                    goto corruptHeader;

                // read entry size (compressed)
                if ( !this->readData( pPtr->fileHandle(), Long, tmp ))
                    goto corruptHeader;
                else
                    pEntryPtr->setLengthInPackage( tmp );

                // read entry size (uncompressed)
                if ( !this->readData( pPtr->fileHandle(), Long, tmp ))
                    goto corruptHeader;
                else
                    pEntryPtr->setLength( tmp );

                // entry size
                if ( !this->readData( pPtr->fileHandle(), Short, entryNameSize ))
                    goto corruptHeader;

                // extra field (skip)
                if ( !this->readData( pPtr->fileHandle(), Short, skip ))
                    goto corruptHeader;

                // read entry name byte by byte
                eName.clear();
                for ( k = 0; k < static_cast<int>( entryNameSize ); k++ ) {
                    if ( !this->readData( pPtr->fileHandle(), Byte, c ))
                        goto corruptHeader;
                    eName.append( QChar( c ));
                }
                pEntryPtr->setName( eName );

                // file or dir?
                if ( pEntryPtr->name().endsWith( "/" ))
                    pEntryPtr->setType( pEntry::Directory );
                else
                    pEntryPtr->setType( pEntry::File );

                // set data offset
                if ( pEntryPtr->lengthInPackage() > 0 )
                    pEntryPtr->setOffset( y + Package::OffsetHeaderEntryName + entryNameSize + skip );
                else
                    pEntryPtr->resetOffset();

                // skip until next header
                if ( !fs.seek( pPtr->fileHandle(), y + Package::OffsetHeaderEntryName + entryNameSize + skip + pEntryPtr->lengthInPackage(), Sys_Filesystem::Silent ))
                    goto corruptHeader;

                // set new offset
                y += Package::OffsetHeaderEntryName + entryNameSize + skip + pEntryPtr->lengthInPackage();

                // add file to package and go on
                pPtr->addFile( pEntryPtr );
                continue;

                // in case we fail
                corruptHeader:
                    delete pEntryPtr;
                com.error( Sys_Common::SoftError, this->tr( "Sys_Package::readPackage: corrupt header in package \"%1\"\n" ).arg( pPtr->name()));
                break;
            } else if ( tmp == Package::IdentCentralDir )
                // we don't really care about central dir headers
                // assuming package is compressed in regular way
                return true;
            else
                com.error( Sys_Common::SoftError, this->tr( "Sys_Package::readPackage: ident mismatch in package \"%1\"\n" ).arg( pPtr->name()));
        } else
            com.error( Sys_Common::SoftError, this->tr( "Sys_Package::readPackage: could not read package \"%1\" ident\n" ).arg( pPtr->name()));

        break;
    }

    // fail
    return false;
}

/*
============
load
============
*/
pPackage *Sys_Package::load( const QString &filename, int flags ) {
    long len;
    fileHandle_t handle;

    // allocate new package
    pPackage *pPtr = new pPackage( filename );

    // read file
    len = fs.open( pFile::Read, filename, handle, Sys_Filesystem::DirsOnly );

    if ( len > 0 ) {
        // set package handle
        pPtr->setFileHandle( handle );

        // read package entries and build file list
        if ( !this->readPackage( pPtr ))
            goto fail;
        else {
            // success, add package to list
            // this becomes a searchpath in filesystem
            // no need to close file, packages stay open until shutdown
            this->packageList << pPtr;
            return pPtr;
        }
    } else
        com.error( Sys_Common::SoftError, this->tr( "Sys_Package::load: could not read package \"%1\"\n" ).arg( filename ));

    // this package ain't worth adding to list
    fail:
        fs.close( handle, ( Sys_Filesystem::OpenFlags )flags );
    delete pPtr;
    return NULL;
}

/*
============
find
============
*/
pEntry *Sys_Package::find( const QString &filename ) {
    foreach ( pPackage *pPtr, this->packageList ) {
        foreach ( pEntry *pEntryPtr, pPtr->fileList ) {
            if ( !QString::compare( pEntryPtr->name(), filename, Qt::CaseInsensitive ))
                return pEntryPtr;
        }
    }
    return NULL;
}

/*
============
shutdown
============
*/
void Sys_Package::shutdown() {
    // clear all data
    foreach ( pPackage *pPtr, this->packageList ) {
        foreach ( pEntry *pEntryPtr, pPtr->fileList ) {
            pEntryPtr->close();
            delete pEntryPtr;
        }

        // close all packages
        fs.close( pPtr->fileHandle());

        // clear all files
        pPtr->fileList.clear();
        delete pPtr;
    }
    this->packageList.clear();
}
