/*
===========================================================================
Original software:
===========================================================================

Copyright (c) 1998, Gilles Vollant
All rights reserved.

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
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

//
// Quick backport from YP2.8
//

//
// includes
//
#include "sys_common.h"
#include "sys_package.h"

//
// classes
//
extern class Sys_Common com;
class Sys_Package pkg;

/*
================
getByte
================
*/
int Sys_Package::getByte( FILE *fin, int *pi ) {
    byte c;
    int err;

    err = fread( &c, 1, 1, fin );

    if ( err == 1 ) {
        *pi = ( int )c;
        return UNZ_OK;
    } else {
        if ( ferror( fin ))
            return Z_ERRNO;
        else
            return UNZ_EOF;
    }
}

/*
================
getShort
================
*/
int Sys_Package::getShort( FILE* fin, uLong *pX ) {
    uLong x;
    int i;
    int err;

    i = 0;
    err = getByte( fin, &i );
    x = ( uLong )i;

    if ( err == UNZ_OK )
        err = getByte( fin, &i );

    x += (( uLong )i ) << 8;

    if ( err == UNZ_OK )
        *pX = x;
    else
        *pX = 0;

    return err;
}

/*
================
getLong
================
*/
int Sys_Package::getLong( FILE* fin, uLong *pX ) {
    uLong x;
    int i;
    int err;

    i = 0;
    err = getByte( fin, &i );
    x = ( uLong )i;

    if ( err == UNZ_OK )
        err = getByte( fin, &i );

    x += (( uLong )i ) << 8;

    if ( err == UNZ_OK )
        err = getByte( fin, &i );

    x += (( uLong )i ) << 16;

    if ( err == UNZ_OK )
        err = getByte( fin, &i );

    x += (( uLong )i ) << 24;

    if ( err == UNZ_OK )
        *pX = x;
    else
        *pX = 0;

    return err;
}

/*
================
searchCentralDir
================
*/
uLong Sys_Package::searchCentralDir( FILE *fin ) {
    byte *buf;
    uLong uSizeFile;
    uLong uBackRead;
    uLong uMaxBack;
    uLong uPosFound;

    uMaxBack = 0xffff;
    uPosFound = 0;

    if ( fseek( fin, 0, SEEK_END ) != 0 )
        return 0;


    uSizeFile = ftell( fin );

    if ( uMaxBack > uSizeFile )
        uMaxBack = uSizeFile;

    buf = new byte[BUFREADCOMMENT + 4];
    if ( buf == NULL )
        return 0;

    uBackRead = 4;

    while ( uBackRead < uMaxBack ) {
        uLong uReadSize, uReadPos;
        int i;

        if ( uBackRead + BUFREADCOMMENT > uMaxBack )
            uBackRead = uMaxBack;
        else
            uBackRead += BUFREADCOMMENT;
        uReadPos = uSizeFile - uBackRead;

        uReadSize = (( BUFREADCOMMENT + 4 ) < ( uSizeFile - uReadPos )) ? ( BUFREADCOMMENT + 4 ) : ( uSizeFile - uReadPos );

        if ( fseek( fin, uReadPos, SEEK_SET ) !=0 )
            break;

        if ( fread( buf, ( uInt )uReadSize, 1, fin ) !=1 )
            break;

        for ( i = ( int )uReadSize-3; ( i--)> 0; ) {
            if (((*( buf + i )) == 0x50 ) && ((*( buf+i+1 )) == 0x4b ) && ((*( buf + i + 2 )) == 0x05 ) && ((*( buf + i + 3)) == 0x06 )) {
                uPosFound = uReadPos + i;
                break;
            }
        }

        if ( uPosFound != 0 )
            break;
    }

    delete buf;
    return uPosFound;
}

/*
================
open
================
*/
pkgFile Sys_Package::open( const char *path ) {
    unz_t us;
    unz_t *s;
    uLong central_pos, uL;
    FILE * fin;
    int err;

    uLong number_disk;
    uLong number_disk_with_CD;
    uLong number_entry_CD;
    err = UNZ_OK;

    fin = fopen( path, "rb" );
    if ( fin == NULL )
        return NULL;

    central_pos = searchCentralDir( fin );
    if ( central_pos == 0 )
        err = Z_ERRNO;

    if ( fseek( fin, central_pos, SEEK_SET ) != 0 )
        err = Z_ERRNO;

    // the signature, already checked
    if ( getLong( fin, &uL ) != UNZ_OK )
        err = Z_ERRNO;

    // number of this disk
    if ( getShort( fin, &number_disk ) != UNZ_OK )
        err = Z_ERRNO;

    // number of the disk with the start of the central directory
    if ( getShort( fin, &number_disk_with_CD ) != UNZ_OK )
        err = Z_ERRNO;

    // total number of entries in the central dir on this disk
    if ( getShort( fin, &us.gi.number_entry ) != UNZ_OK )
        err = Z_ERRNO;

    // total number of entries in the central dir
    if ( getShort( fin, &number_entry_CD ) != UNZ_OK )
        err = Z_ERRNO;

    if (( number_entry_CD != us.gi.number_entry ) || ( number_disk_with_CD != 0 ) || ( number_disk != 0 ))
        err = UNZ_BADZIPFILE;

    // size of the central directory
    if ( getLong( fin, &us.size_central_dir ) != UNZ_OK )
        err = Z_ERRNO;

    // offset of start of central directory with respect to the starting disk number
    if ( getLong( fin, &us.offset_central_dir ) != UNZ_OK )
        err = Z_ERRNO;

    /* zipfile comment length */
    if ( getShort( fin, &us.gi.size_comment ) != UNZ_OK )
        err = Z_ERRNO;

    if (( central_pos < us.offset_central_dir + us.size_central_dir ) && ( err == UNZ_OK ))
        err = UNZ_BADZIPFILE;

    if ( err != UNZ_OK ) {
        fclose( fin );
        return NULL;
    }

    us.file = fin;
    us.byte_before_the_zipfile = central_pos - ( us.offset_central_dir + us.size_central_dir );
    us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;

    s = new unz_t;
    *s = us;

    return ( pkgFile )s;
}

/*
================
close
================
*/
int Sys_Package::close( pkgFile file ) {
    unz_t* s;

    if ( file == NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;

    if ( s->pfile_in_zip_read != NULL )
        closeCurrentFile( file );

    fclose( s->file );
    delete s;

    return UNZ_OK;
}

/*
================
getGlobalInfo
================
*/
int Sys_Package::getGlobalInfo( pkgFile file, globalInfo_t *pglobal_info ) {
    unz_t* s;

    if ( file == NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;

    *pglobal_info = s->gi;

    return UNZ_OK;
}


/*
================
legacyDateToTmuDate
================
*/
void Sys_Package::legacyDateToTmuDate( uLong ulDosDate, pkgTime_t* ptm ) {
    uLong uDate;

    uDate = ( uLong )( ulDosDate >> 16 );
    ptm->tm_mday = ( uInt )( uDate & 0x1f );
    ptm->tm_mon =  ( uInt )(((( uDate ) & 0x1E0 ) / 0x20 ) - 1 );
    ptm->tm_year = ( uInt )((( uDate & 0x0FE00 ) / 0x0200 ) + 1980 );
    ptm->tm_hour = ( uInt ) (( ulDosDate & 0xF800 ) / 0x800 );
    ptm->tm_min =  ( uInt ) (( ulDosDate&0x7E0 ) / 0x20 );
    ptm->tm_sec =  ( uInt ) ( 2 * ( ulDosDate & 0x1f ));
}

/*
================
getCurrentFileInfoInternal
================
*/
int Sys_Package::getCurrentFileInfoInternal( pkgFile file, fileInfo_t *pfile_info, fileInfoInternal_t *pfile_info_internal, char *szFileName, uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize, char *szComment, uLong commentBufferSize ) {
    unz_t* s;
    fileInfo_t file_info;
    fileInfoInternal_t file_info_internal;
    int err;
    uLong uMagic;
    long lSeek;

    lSeek = 0;
    err = UNZ_OK;

    if ( file == NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;
    if ( fseek( s->file, s->pos_in_central_dir + s->byte_before_the_zipfile, SEEK_SET ) !=0 )
        err = Z_ERRNO;

    // check the magic
    if ( err == UNZ_OK ) {
        if ( getLong( s->file, &uMagic ) != UNZ_OK )
            err = Z_ERRNO;
        else if ( uMagic != 0x02014b50 )
            err = UNZ_BADZIPFILE;
        }

    if ( getShort( s->file, &file_info.version ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.version_needed ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.flag ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.compression_method ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getLong( s->file, &file_info.dosDate ) != UNZ_OK )
        err = Z_ERRNO;

    legacyDateToTmuDate( file_info.dosDate, &file_info.tmu_date );

    if ( getLong( s->file, &file_info.crc ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getLong( s->file, &file_info.compressed_size ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getLong( s->file, &file_info.uncompressed_size ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.size_filename ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.size_file_extra ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.size_file_comment ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.disk_num_start ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &file_info.internal_fa ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getLong( s->file, &file_info.external_fa ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getLong( s->file, &file_info_internal.offset_curfile ) != UNZ_OK )
        err = Z_ERRNO;

    lSeek += file_info.size_filename;
    if (( err == UNZ_OK ) && ( szFileName != NULL ))  {
        uLong uSizeRead;

        if ( file_info.size_filename < fileNameBufferSize ) {
            *( szFileName + file_info.size_filename ) = '\0';
            uSizeRead = file_info.size_filename;
        } else
            uSizeRead = fileNameBufferSize;

        if (( file_info.size_filename > 0 ) && ( fileNameBufferSize > 0 ))
            if ( fread( szFileName, ( uInt )uSizeRead, 1, s->file ) != 1 )
                err = Z_ERRNO;

        lSeek -= uSizeRead;
    }


    if (( err == UNZ_OK ) && ( extraField!= NULL )) {
        uLong uSizeRead;
        if ( file_info.size_file_extra<extraFieldBufferSize )
            uSizeRead = file_info.size_file_extra;
        else
            uSizeRead = extraFieldBufferSize;

        if ( lSeek != 0 ) {
            if ( fseek( s->file, lSeek, SEEK_CUR ) == 0 )
                lSeek = 0;
            else
                err = Z_ERRNO;
            }

        if (( file_info.size_file_extra > 0 ) && ( extraFieldBufferSize > 0 ))
            if ( fread( extraField, ( uInt )uSizeRead, 1, s->file ) != 1 )
                err = Z_ERRNO;
        lSeek += file_info.size_file_extra - uSizeRead;
    } else
        lSeek += file_info.size_file_extra;


    if (( err == UNZ_OK ) && ( szComment!= NULL )) {
        uLong uSizeRead;

        if ( file_info.size_file_comment < commentBufferSize ) {
            *( szComment + file_info.size_file_comment ) = '\0';
            uSizeRead = file_info.size_file_comment;
        } else
            uSizeRead = commentBufferSize;

        if ( lSeek != 0 ) {
            if ( fseek( s->file, lSeek, SEEK_CUR ) == 0 )
                lSeek = 0;
            else
                err = Z_ERRNO;
        }

        if (( file_info.size_file_comment> 0 ) && (commentBufferSize> 0 ))
            if ( fread( szComment, ( uInt )uSizeRead, 1, s->file ) != 1 )
                err = Z_ERRNO;

        lSeek += file_info.size_file_comment - uSizeRead;
    } else
        lSeek += file_info.size_file_comment;

    if (( err == UNZ_OK ) && ( pfile_info!= NULL ))
        *pfile_info = file_info;

    if (( err == UNZ_OK ) && ( pfile_info_internal != NULL ))
        *pfile_info_internal = file_info_internal;

    return err;
}

/*
================
getCurrentFileInfo
================
*/
int Sys_Package::getCurrentFileInfo( pkgFile file, fileInfo_t *pfile_info, char *szFileName, uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize, char *szComment, uLong commentBufferSize ) {
    return getCurrentFileInfoInternal( file, pfile_info, NULL, szFileName, fileNameBufferSize, extraField, extraFieldBufferSize, szComment, commentBufferSize );
}

/*
================
checkCurrentFileCoherencyHeader
================
*/
int Sys_Package::checkCurrentFileCoherencyHeader( unz_t* s, uInt* piSizeVar, uLong *poffset_local_extrafield, uInt  *psize_local_extrafield ) {
    uLong uMagic, uData, uFlags;
    uLong size_filename;
    uLong size_extra_field;
    int err = UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    if ( fseek( s->file, s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile, SEEK_SET ) != 0 )
        return Z_ERRNO;

    if ( err == UNZ_OK ) {
        if ( getLong( s->file, &uMagic ) != UNZ_OK )
            err = Z_ERRNO;
        else if ( uMagic!=0x04034b50 )
            err = UNZ_BADZIPFILE;
    }

    if ( getShort( s->file, &uData ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &uFlags ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getShort( s->file, &uData ) != UNZ_OK )
        err = Z_ERRNO;

    else if (( err == UNZ_OK ) && ( uData != s->cur_file_info.compression_method ))
        err = UNZ_BADZIPFILE;

    if (( err == UNZ_OK ) && ( s->cur_file_info.compression_method != 0 ) && ( s->cur_file_info.compression_method != Z_DEFLATED ))
        err = UNZ_BADZIPFILE;

    if ( getLong( s->file, &uData ) != UNZ_OK )
        err = Z_ERRNO;

    if ( getLong( s->file, &uData ) != UNZ_OK )
        err = Z_ERRNO;
    else if (( err == UNZ_OK ) && ( uData != s->cur_file_info.crc ) &&
             (( uFlags & 8) == 0 ))
        err = UNZ_BADZIPFILE;

    if ( getLong( s->file, &uData ) != UNZ_OK )
        err = Z_ERRNO;
    else if (( err == UNZ_OK ) && ( uData != s->cur_file_info.compressed_size ) &&
             (( uFlags & 8) == 0 ))
        err = UNZ_BADZIPFILE;

    if ( getLong( s->file, &uData ) != UNZ_OK ) /* size uncompr */
        err = Z_ERRNO;
    else if (( err == UNZ_OK ) && ( uData != s->cur_file_info.uncompressed_size ) &&
             (( uFlags & 8) == 0 ))
        err = UNZ_BADZIPFILE;

    if ( getShort( s->file, &size_filename ) != UNZ_OK )
        err = Z_ERRNO;
    else if (( err == UNZ_OK ) && ( size_filename != s->cur_file_info.size_filename ))
        err = UNZ_BADZIPFILE;

    *piSizeVar += ( uInt )size_filename;

    if ( getShort( s->file, &size_extra_field ) != UNZ_OK )
        err = Z_ERRNO;

    *poffset_local_extrafield = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = ( uInt )size_extra_field;
    *piSizeVar += ( uInt )size_extra_field;

    return err;
}

/*
================
unzOpenCurrentFile
================
*/
int Sys_Package::openCurrentFile( pkgFile file ) {
    int err = UNZ_OK;
    int store;
    uInt iSizeVar;
    unz_t* s;
    fileInPackageReadInfo_t* pFileReadInfo;
    uLong offset_local_extrafield;
    uInt  size_local_extrafield;

    if ( file== NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t*)file;

    if ( !s->current_file_ok )
        return UNZ_PARAMERROR;

    if ( s->pfile_in_zip_read != NULL )
        closeCurrentFile( file );

    if ( checkCurrentFileCoherencyHeader( s, &iSizeVar, &offset_local_extrafield, &size_local_extrafield ) !=UNZ_OK )
        return UNZ_BADZIPFILE;

    pFileReadInfo = new fileInPackageReadInfo_t;
    if ( pFileReadInfo== NULL )
        return UNZ_INTERNALERROR;

    pFileReadInfo->read_buffer = new char[UNZ_BUFSIZE];
    pFileReadInfo->offset_local_extrafield = offset_local_extrafield;
    pFileReadInfo->size_local_extrafield = size_local_extrafield;
    pFileReadInfo->pos_local_extrafield = 0;

    if ( pFileReadInfo->read_buffer == NULL ) {
        delete pFileReadInfo;
        return UNZ_INTERNALERROR;
    }

    pFileReadInfo->stream_initialised = 0;

    if (( s->cur_file_info.compression_method!= 0 ) && ( s->cur_file_info.compression_method != Z_DEFLATED ))
        err = UNZ_BADZIPFILE;

    store = s->cur_file_info.compression_method == 0;

    pFileReadInfo->crc32_wait = s->cur_file_info.crc;
    pFileReadInfo->crc32 = 0;
    pFileReadInfo->compression_method = s->cur_file_info.compression_method;
    pFileReadInfo->file = s->file;
    pFileReadInfo->byte_before_the_zipfile = s->byte_before_the_zipfile;

    pFileReadInfo->stream.total_out = 0;

    if ( !store ) {
        pFileReadInfo->stream.zalloc = ( alloc_func )0;
        pFileReadInfo->stream.zfree = ( free_func )0;
        pFileReadInfo->stream.opaque = ( voidpf )0;

        err=inflateInit2( &pFileReadInfo->stream, -MAX_WBITS);

        if ( err == Z_OK )
            pFileReadInfo->stream_initialised = 1;
    }

    pFileReadInfo->rest_read_compressed = s->cur_file_info.compressed_size;
    pFileReadInfo->rest_read_uncompressed = s->cur_file_info.uncompressed_size;


    pFileReadInfo->pos_in_zipfile = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar;
    pFileReadInfo->stream.avail_in = ( uInt )0;

    s->pfile_in_zip_read = pFileReadInfo;
    return UNZ_OK;
}

/*
================
unzReadCurrentFile
================
*/
int Sys_Package::readCurrentFile( pkgFile file, void* buf, unsigned len ) {
    int err = UNZ_OK;
    uInt iRead = 0;
    unz_t* s;
    fileInPackageReadInfo_t* pFileReadInfo;

    if ( file== NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;
    pFileReadInfo = s->pfile_in_zip_read;

    if ( pFileReadInfo == NULL )
        return UNZ_PARAMERROR;

    if (( pFileReadInfo->read_buffer == NULL ))
        return UNZ_EOLF;

    if ( len == 0 )
        return 0;

    pFileReadInfo->stream.next_out = ( Bytef * )buf;
    pFileReadInfo->stream.avail_out = ( uInt )len;

    if ( len>pFileReadInfo->rest_read_uncompressed )
        pFileReadInfo->stream.avail_out = ( uInt )pFileReadInfo->rest_read_uncompressed;

    while ( pFileReadInfo->stream.avail_out > 0 ) {
        if (( pFileReadInfo->stream.avail_in == 0 ) && ( pFileReadInfo->rest_read_compressed > 0 )) {
            uInt uReadThis = UNZ_BUFSIZE;

            if ( pFileReadInfo->rest_read_compressed<uReadThis )
                uReadThis = ( uInt )pFileReadInfo->rest_read_compressed;

            if ( uReadThis == 0 )
                return UNZ_EOF;

            if ( fseek( pFileReadInfo->file, pFileReadInfo->pos_in_zipfile + pFileReadInfo->byte_before_the_zipfile,SEEK_SET ) != 0 )
                return Z_ERRNO;

            if ( fread( pFileReadInfo->read_buffer, uReadThis, 1, pFileReadInfo->file ) != 1 )
                return Z_ERRNO;

            pFileReadInfo->pos_in_zipfile += uReadThis;
            pFileReadInfo->rest_read_compressed-=uReadThis;
            pFileReadInfo->stream.next_in = (Bytef*)pFileReadInfo->read_buffer;
            pFileReadInfo->stream.avail_in = ( uInt )uReadThis;
        }

        if ( pFileReadInfo->compression_method == 0 ) {
            uInt uDoCopy, i;
            if ( pFileReadInfo->stream.avail_out < pFileReadInfo->stream.avail_in )
                uDoCopy = pFileReadInfo->stream.avail_out;
            else
                uDoCopy = pFileReadInfo->stream.avail_in;

            for ( i = 0; i < uDoCopy; i++ )
                *( pFileReadInfo->stream.next_out+i) = *( pFileReadInfo->stream.next_in+i);

            pFileReadInfo->rest_read_uncompressed-=uDoCopy;
            pFileReadInfo->stream.avail_in -= uDoCopy;
            pFileReadInfo->stream.avail_out -= uDoCopy;
            pFileReadInfo->stream.next_out += uDoCopy;
            pFileReadInfo->stream.next_in += uDoCopy;
            pFileReadInfo->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        } else {
            uLong uTotalOutBefore, uTotalOutAfter;
            const Bytef *bufBefore;
            uLong uOutThis;
            int flush = Z_SYNC_FLUSH;

            uTotalOutBefore = pFileReadInfo->stream.total_out;
            bufBefore = pFileReadInfo->stream.next_out;
            err = inflate( &pFileReadInfo->stream, flush );
            uTotalOutAfter = pFileReadInfo->stream.total_out;
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pFileReadInfo->rest_read_uncompressed -= uOutThis;
            iRead += ( uInt )( uTotalOutAfter - uTotalOutBefore );

            if ( err == Z_STREAM_END )
                return ( iRead == 0 ) ? UNZ_EOF : iRead;
            if ( err != Z_OK)
                break;
        }
    }

    if ( err == Z_OK )
        return iRead;

    return err;
}

/*
================
closeCurrentFile
================
*/
int Sys_Package::closeCurrentFile( pkgFile file ) {
    int err = UNZ_OK;

    unz_t* s;
    fileInPackageReadInfo_t* pFileReadInfo;

    if ( file== NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;
    pFileReadInfo = s->pfile_in_zip_read;

    if ( pFileReadInfo == NULL )
        return UNZ_PARAMERROR;

    delete pFileReadInfo->read_buffer;
    pFileReadInfo->read_buffer = NULL;

    if ( pFileReadInfo->stream_initialised )
        inflateEnd( &pFileReadInfo->stream );

    pFileReadInfo->stream_initialised = 0;
    delete pFileReadInfo;

    s->pfile_in_zip_read = NULL;

    return err;
}

/*
================
getCurrentFileInfoPosition
================
*/
int Sys_Package::getCurrentFileInfoPosition( pkgFile file, unsigned long *pos ) {
    unz_t* s;

    if ( file== NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;

    *pos = s->pos_in_central_dir;
    return UNZ_OK;
}

/*
================
setCurrentFileInfoPosition
================
*/
int Sys_Package::setCurrentFileInfoPosition( pkgFile file, unsigned long pos ) {
    unz_t* s;
    int err;

    if ( file== NULL )

        return UNZ_PARAMERROR;
    s = ( unz_t* )file;

    s->pos_in_central_dir = pos;
    err = getCurrentFileInfoInternal( file, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0 );
    s->current_file_ok = ( err == UNZ_OK );

    return UNZ_OK;
}

/*
================
goToFirstFile
================
*/
int Sys_Package::goToFirstFile( pkgFile file ) {
    int err = UNZ_OK;
    unz_t* s;

    if ( file == NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;
    s->pos_in_central_dir = s->offset_central_dir;
    s->num_file = 0;
    err = getCurrentFileInfoInternal(file, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0 );
    s->current_file_ok = ( err == UNZ_OK );

    return err;
}

/*
================
goToNextFile
================
*/
int Sys_Package::goToNextFile( pkgFile file ) {
    unz_t* s;
    int err;

    if ( file == NULL )
        return UNZ_PARAMERROR;

    s = ( unz_t* )file;
    if ( !s->current_file_ok )
        return UNZ_EOLF;

    if ( s->num_file + 1 == s->gi.number_entry )
        return UNZ_EOLF;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename + s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment;
    s->num_file++;
    err = getCurrentFileInfoInternal( file,&s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0 );
    s->current_file_ok = ( err == UNZ_OK );

    return err;
}

/*
================
fTell
================
*/
long Sys_Package::fTell( pkgFile file ) {
    unz_s* s;
    fileInPackageReadInfo_t* pFileReadInfo;

    if ( file == NULL )
        return UNZ_PARAMERROR;

    s = ( unz_s* )file;
    pFileReadInfo = s->pfile_in_zip_read;

    if ( pFileReadInfo == NULL )
        return UNZ_PARAMERROR;

    return (long)pFileReadInfo->stream.total_out;
}

