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
C++ port for YPlatform2:
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

#ifndef SYSPACKAGE_H
#define SYSPACKAGE_H

//
// includes
//
#include "sys_shared.h"
#include <zlib.h>

//
// defines
//
typedef void *pkgFile;

#define UNZ_OK                 0
#define UNZ_EOF                0
#define UNZ_EOLF            -100
#define UNZ_PARAMERROR      -102
#define UNZ_BADZIPFILE      -103
#define UNZ_INTERNALERROR   -104
#define UNZ_CRCERROR        -105

//
// defines
//
#define UNZ_BUFSIZE             16384
#define UNZ_MAXFILENAMEINZIP    256
#define SIZECENTRALDIRITEM      0x2e
#define SIZEZIPLOCALHEADER      0x1e
#define BUFREADCOMMENT          0x400

// pkgTime_t contain date/time info
typedef struct pkgTime_s  {
    uInt tm_sec;
    uInt tm_min;
    uInt tm_hour;
    uInt tm_mday;
    uInt tm_mon;
    uInt tm_year;
} pkgTime_t;

// global data about the package
typedef struct globalInfo_s {
    uLong number_entry;
    uLong size_comment;
} globalInfo_t;

// information about a file in the zipfile
typedef struct fileInfo_s {
    uLong version;
    uLong version_needed;
    uLong flag;
    uLong compression_method;
    uLong dosDate;
    uLong crc;
    uLong compressed_size;
    uLong uncompressed_size;
    uLong size_filename;
    uLong size_file_extra;
    uLong size_file_comment;
    uLong disk_num_start;
    uLong internal_fa;
    uLong external_fa;
    pkgTime_t tmu_date;
} fileInfo_t;

// internal info about a file in package
typedef struct fileInfoInternal_s {
    uLong offset_curfile;
} fileInfoInternal_t;

// internal information about a file in zipfile when reading and decompressing it
typedef struct fileInPackageReadInfo_s {
    char *read_buffer;
    z_stream stream;
    uLong pos_in_zipfile;
    uLong stream_initialised;
    uLong offset_local_extrafield;
    uInt  size_local_extrafield;
    uLong pos_local_extrafield;
    uLong crc32;
    uLong crc32_wait;
    uLong rest_read_compressed;
    uLong rest_read_uncompressed;
    FILE *file;
    uLong compression_method;
    uLong byte_before_the_zipfile;
} fileInPackageReadInfo_t;

// unz struct
typedef struct unz_s {
    FILE *file;
    globalInfo_t gi;
    uLong byte_before_the_zipfile;
    uLong num_file;
    uLong pos_in_central_dir;
    uLong current_file_ok;
    uLong central_pos;
    uLong size_central_dir;
    uLong offset_central_dir;
    fileInfo_t cur_file_info;
    fileInfoInternal_t cur_file_info_internal;
    fileInPackageReadInfo_t *pfile_in_zip_read;
} unz_t;

//
// class::Sys_Package
//
class Sys_Package : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem package support class" )

private:
    int getByte( FILE*, int* );
    int getShort( FILE*, uLong* );
    int getLong( FILE*, uLong* );
    uLong searchCentralDir( FILE* );
    void legacyDateToTmuDate( uLong, pkgTime_t* );
    int getCurrentFileInfoInternal( pkgFile, fileInfo_t *, fileInfoInternal_t *, char*, uLong, void*, uLong extraFieldBufferSize, char*, uLong );
    int checkCurrentFileCoherencyHeader( unz_t*, uInt*, uLong*, uInt* );

public:
    pkgFile open( const char* );
    int close( pkgFile );
    int getGlobalInfo( pkgFile, globalInfo_t* );
    int getCurrentFileInfo( pkgFile, fileInfo_t *, char*, uLong, void*, uLong, char*, uLong );
    int openCurrentFile( pkgFile );
    int closeCurrentFile( pkgFile );
    int readCurrentFile( pkgFile, void*, unsigned );
    int getCurrentFileInfoPosition( pkgFile, unsigned long* );
    int setCurrentFileInfoPosition( pkgFile, unsigned long );
    int goToFirstFile( pkgFile );
    int goToNextFile( pkgFile );
    long fTell( pkgFile );
};

#endif
