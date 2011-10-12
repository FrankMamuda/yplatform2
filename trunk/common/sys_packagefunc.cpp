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
// includes
//
#include "sys_common.h"
#include "sys_packagefunc.h"
#include "sys_package.h"
#include "sys_filesystem.h"

/*
============
construct
============
*/
pPackage::pPackage( const QString &filename, QObject *parent ) : QObject( parent ) {
    this->setName( filename );
}

/*
============
addFile
============
*/
void pPackage::addFile( pEntry *pEntryPtr ) {
    // set package parent
    pEntryPtr->setParent( this );

    // add to list
    this->fileList << pEntryPtr;
}

/*
============
open
============
*/
long pEntry::open() {
    // close first if already open
    if ( this->hasInitialized())
        this->close();

    // init zlib stream (make sure we reset stream)
    this->resetStream();
    if ( inflateInit2( &this->stream, -MAX_WBITS ) != Z_OK )
        return -1;

    // make sure we read from the start
    this->setInitialized();
    this->resetRemaining();
    this->resetPos();

    // return uncompressed size
    return this->length();
}

/*
============
close
============
*/
void pEntry::close() {
    if ( !this->hasInitialized())
        return;

    // close zlib stream
    inflateEnd( &this->stream );
    this->setInitialized( false );

    // make sure we read from the start
    delete this->readBuffer;
}

/*
============
read
============
*/
long pEntry::read( byte *buffer, unsigned long len, int flags ) {
    // failsafe
    if ( !this->hasInitialized()) {
        com.error( Sys_Common::SoftError, this->tr( "pEntry( '%1' )::read: file has not been opened\n" ).arg( this->name()));
        return -1;
    }

    // set read params, bind buffer to stream
    this->stream.next_out = buffer;
    this->stream.avail_out = static_cast<unsigned int>( len );

    // failsafe, can't read more than uncompressed length
    if (( unsigned long )len > this->length())
        this->stream.avail_out = static_cast<unsigned int>( this->length());

    // allocate read buffer
    this->readBuffer = new byte[Package::ReadBuffer];

    // enter read loop
    while ( this->stream.avail_out > 0 ) {
        if (( this->stream.avail_in == 0 ) && ( this->remaining() > 0 )) {
            unsigned int readBufferSize = Package::ReadBuffer;

            // set read buffer size
            if ( this->remaining() < readBufferSize )
                readBufferSize = static_cast<unsigned int>( this->remaining());

            // failsafe
            if ( readBufferSize == 0 )
                return -1;

            // seek until compressed data
            if ( !fs.seek( this->parent()->fileHandle(), this->offset() + this->pos(), static_cast<Sys_Filesystem::OpenFlags>( flags )))
                return -1;

            // read compressed data with buffer size
            if ( !fs.read( readBuffer, readBufferSize, this->parent()->fileHandle(), static_cast<Sys_Filesystem::OpenFlags>( flags )))
                return -1;

            // advance by number of bytes read
            this->setPos( this->pos() + readBufferSize );
            this->setRemaining( this->remaining() - readBufferSize );
            this->stream.next_in = readBuffer;
            this->stream.avail_in = static_cast<unsigned int>( readBufferSize );
        }

        // if data is uncompressed, just copy it
        // if data is compressed with deflate algorithm, inflate it
        if ( this->compressionMode() == pEntry::NoCompression ) {
            unsigned int copyLength, y;

            if ( this->stream.avail_out < this->stream.avail_in )
                copyLength = this->stream.avail_out;
            else
                copyLength = this->stream.avail_in;

            // perform copy
            for ( y = 0; y < copyLength; y++ )
                *( this->stream.next_out + y ) = *( this->stream.next_in + y );

            // advance
            this->setRemaining( this->remaining() - copyLength );
            this->stream.avail_in -= copyLength;
            this->stream.avail_out -= copyLength;
            this->stream.next_out += copyLength;
            this->stream.next_in += copyLength;
            this->stream.total_out += copyLength;
        } else if ( this->compressionMode() == pEntry::Compressed ) {
            unsigned long bytesBefore, bytesAfter, bytesTotal;

            // perform inflation
            bytesBefore = this->stream.total_out;
            if ( inflate( &this->stream, Z_SYNC_FLUSH ) != Z_OK )
                return -1;

            bytesAfter = this->stream.total_out;
            bytesTotal = bytesBefore - bytesAfter;

            // advance
            this->setRemaining( this->remaining() - bytesTotal );
        } else {
            com.error( Sys_Common::SoftError, this->tr( "pEntry( '%1' )::read: unsupported compression method\n" ).arg( this->name()));
            return -1;
        }
    }
    return len;
}
