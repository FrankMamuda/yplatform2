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

#ifndef SYS_PACKAGEFUNC_H
#define SYS_PACKAGEFUNC_H

//
// includes
//
#include "sys_shared.h"
#include "sys_filesystem.h"
#include <zlib.h>

//
// class:pEntry
//
class pPackage;
class pEntry : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem package entry" )
    Q_ENUMS( Types )
    Q_ENUMS( CompressionModes )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( CompressionModes compression READ compressionMode WRITE setCompressionMode )
    Q_PROPERTY( unsigned long offset READ offset WRITE setOffset RESET resetOffset )
    Q_PROPERTY( unsigned long length READ length WRITE setLength )
    Q_PROPERTY( unsigned long lengthInPackage READ lengthInPackage WRITE setLengthInPackage )
    Q_PROPERTY( unsigned long pos READ pos WRITE setPos RESET resetPos )
    Q_PROPERTY( unsigned long remaining READ remaining WRITE setRemaining RESET resetRemaining )
    Q_PROPERTY( pPackage *parent READ parent WRITE setParent )

public:
    enum Types {
        File = 0,
        Directory
    };
    enum CompressionModes {
        NoCompression = 0,
        Compressed
    };

    // property setters
    QString name() const { return this->m_name; }
    bool hasInitialized() const { return this->m_initialized; }
    Types type() const { return this->m_type; }
    CompressionModes compressionMode() const { return this->m_compression; }
    unsigned long offset() const { return this->m_offset; }
    unsigned long length() const { return this->m_length; }
    unsigned long lengthInPackage() const { return this->m_lengthInPackage; }
    unsigned long pos() const { return this->m_pos; }
    unsigned long remaining() const { return this->m_remaining; }
    pPackage *parent() { return this->m_parent; }

public slots:
    long open();
    long read( byte *, unsigned long, int flags );
    void close();
    void resetStream() { memset( &this->stream, 0, sizeof( this->stream )); }

    // property getters
    void setName( const QString &name ) { this->m_name = name; }
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void setType( Types type = File ) { this->m_type = type; }
    void setCompressionMode( CompressionModes mode = NoCompression ) { this->m_compression = mode; }
    void setOffset( unsigned long offset ) { this->m_offset = offset; }
    void resetOffset() { this->m_offset = 0; }
    void setLength( unsigned long length ) { this->m_length = length; }
    void setLengthInPackage( unsigned long lengthInPackage ) { this->m_lengthInPackage = lengthInPackage; }
    void setPos( unsigned long pos ) { this->m_pos = pos; }
    void resetPos() { this->m_pos = 0; }
    void setRemaining( unsigned long remaining ) { this->m_remaining = remaining; }
    void resetRemaining() { this->m_remaining = m_lengthInPackage; }
    void setParent( pPackage *parent ) { this->m_parent = parent; }

private:
    z_stream stream;
    byte *readBuffer;

    // properties
    bool m_initialized;
    QString m_name;
    Types m_type;
    CompressionModes m_compression;
    unsigned long m_offset;
    unsigned long m_length;
    unsigned long m_lengthInPackage;
    unsigned long m_pos;
    unsigned long m_remaining;
    pPackage *m_parent;
};

//
// class:pPackage
//
class pPackage : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem package" )
    Q_DISABLE_COPY( pPackage )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( fileHandle_t handle READ fileHandle WRITE setFileHandle )

public:
    explicit pPackage( const QString &, QObject * = 0 );
    QList <pEntry*>fileList;
    QString name() const { return this->m_name; }
    fileHandle_t fileHandle() const { return m_handle; }

public slots:
    void setName( const QString &name ) { this->m_name = name; }
    void setFileHandle( fileHandle_t handle ) { this->m_handle = handle; }
    void addFile( pEntry * );

private:
    QString m_name;
    fileHandle_t m_handle;
};

#endif // SYS_PACKAGEFUNC_H
