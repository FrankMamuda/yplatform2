/*
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
// dd, filesystem complete rewrite as part of YCommons 2.9
//

//
// includes
//
#include "sys_common.h"
#include "sys_filesystem.h"
#include "sys_package.h"
#include "sys_cvar.h"
#include "sys_cmd.h"

//
// classes
//
extern class Sys_Cvar cv;
extern class Sys_Common com;
extern class Sys_Cmd cmd;
extern class Sys_Package pkg;
class Sys_Filesystem fs;

//
// cvars
//
pCvar *fs_homePath;
pCvar *fs_appPath;
pCvar *fs_debug;
pCvar *fs_baseDir;
pCvar *fs_ignoreLinks;

//
// commands
//
createCommand( fs, touch )
createCommand( fs, list )

/*
============
init
============
*/
void Sys_Filesystem::init() {
    // get homeDir
    QString homeDir = QDir::homePath();
    homeDir.append( FS_PATH_SEPARATOR );
    homeDir.append( FS_PLATFORM_DIRECTORY );

    // create cvars
    fs_homePath = cv.create( "fs_homePath", homeDir.replace( '\\', FS_PATH_SEPARATOR ).append( FS_PATH_SEPARATOR ), ( CVAR_ROM ));
    fs_appPath = cv.create( "fs_appPath", QDir::currentPath().replace( '\\', FS_PATH_SEPARATOR ).append( FS_PATH_SEPARATOR ), ( CVAR_ROM ));
    fs_baseDir = cv.create( "fs_baseDir", QString( FS_BASE_DIRECTORY ).append( FS_PATH_SEPARATOR ), ( CVAR_ARCHIVE | CVAR_ROM ));
    fs_debug = cv.create( "fs_debug", "0", CVAR_ARCHIVE );
    fs_ignoreLinks = cv.create( "fs_ignoreLinks", "0", CVAR_ARCHIVE );

    // add searchPaths
    this->addSearchPath( fs_homePath->string().append( fs_baseDir->string()), FS_HOMEPATH_ID );
    this->addSearchPath( fs_appPath->string().append( fs_baseDir->string()), FS_APPPATH_ID );

    // load packages
    this->loadPackages();

    // for debugging
    if ( fs_debug->integer()) {
        foreach ( searchPath_t *sp, this->searchPaths ) {
            if ( sp->type == 0 )
                com.print( this->tr( "^6Sys_Filesystem::init: searchPath: %1 (dir)\n" ).arg( sp->path ));
            else
                com.print( this->tr( "^6Sys_Filesystem::init: searchPath: %1 (package)\n" ).arg( sp->path ));

        }
    }

    // reset handles
    // -1 and 0 is reserved
    this->numFileHandles = 1;

    // add commmands
    cmd.addCommand( "fs_touch", touchCmd, "create an empty file" );
    cmd.addCommand( "fs_list", listCmd, "list contents of a directory" );

    // all done
    this->initialized = true;
}

/*
=================
loadPackage
=================
*/
package_t *Sys_Filesystem::loadPackage( const QString &packageName, int searchPathIndex ) {
    package_t   *pack;
    pkgFile     pHandle;
    int         err;
    globalInfo_t gInfo;
    char filename[FS_PACKAGE_MAXPATH];
    fileInfo_t fileInfo;
    QString pName;
    int i;

    // get full path
    pName = this->searchPaths.at( searchPathIndex )->path + packageName;

    // open package
    pHandle = pkg.open( pName.toLatin1().constData());
    err = pkg.getGlobalInfo( pHandle, &gInfo );

    // failsafe
    if ( err != UNZ_OK )
        return NULL;

    // validate package
    pkg.goToFirstFile( pHandle );
    for ( i = 0; i < (int)gInfo.number_entry; i++ ) {
        err = pkg.getCurrentFileInfo( pHandle, &fileInfo, filename, sizeof( filename ), NULL, 0, NULL, 0 );
        if ( err != UNZ_OK )
            break;

        pkg.goToNextFile( pHandle );
    }

    // allocate a package
    pack = new package_t;
    pack->searchPathIndex = searchPathIndex;
    pack->handle = pHandle;
    pkg.goToFirstFile( pHandle );

    // search trough package, store offsets and names into fileTable
    for ( i = 0; i < (int)gInfo.number_entry; i++ ) {
        // failsafe
        err = pkg.getCurrentFileInfo( pHandle, &fileInfo, filename, sizeof( filename ), NULL, 0, NULL, 0 );
        if ( err != UNZ_OK )
            break;

        // allocate a new packaged file struct
        packedFileInfo_t *pFile = new packedFileInfo_t;
        pFile->name = QString( filename ).toLower();

        // store the file position in the package
        pkg.getCurrentFileInfoPosition( pHandle, &pFile->pos );
        pkg.goToNextFile( pHandle );

        // add packaged file to fileList
        pack->fileList << pFile;
    }

    // return package
    return pack;
}

/*
============
loadPackages
============
*/
void Sys_Filesystem::loadPackages() {
    int y = 0;

    foreach ( searchPath_t *sp, this->searchPaths ) {
        if ( sp->type == SEARCHPATH_DIRECTORY ) {
            QDir dir( sp->path );
            dir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
            QStringList filters;
            filters << QString( "*" ).append( FS_PACKAGE_EXTENSION );
            dir.setNameFilters( filters );
            dir.setSorting( QDir::Name | QDir::Reversed );

            QFileInfoList list = dir.entryInfoList();
            foreach ( QFileInfo fInfo, list ) {
                // we add packages from homePath first in descending order
                this->addSearchPath( this->loadPackage( fInfo.fileName(), y ), fInfo.fileName());
            }
        }
        y++;
    }
}

/*
============
addSearchPath
============
*/
void Sys_Filesystem::addSearchPath( const QString &path, const QString id ) {
    searchPath_t *searchPath = new searchPath_t();
    searchPath->path = path;
    searchPath->id = id;
    searchPath->type = SEARCHPATH_DIRECTORY;
    this->searchPaths << searchPath;
}

/*
============
addSearchPath
============
*/
void Sys_Filesystem::addSearchPath( package_t *package, const QString &filename, const QString id ) {
    searchPath_t *searchPath = new searchPath_t();
    searchPath->package = package;
    searchPath->path = filename;
    searchPath->id = id;
    searchPath->type = SEARCHPATH_PACKAGE;
    this->searchPaths << searchPath;
}

/*
============
getSearchPathIndex
============
*/
int Sys_Filesystem::getSearchPathIndex( const QString &id ) {
    int y = 0;

    foreach ( searchPath_t *sp, this->searchPaths ) {
        if ( !QString::compare( id, sp->id ))
            return y;

        y++;
    }
    return -1;
}

/*
============
buildPath
============
*/
QString Sys_Filesystem::buildPath( const QString &filename, const QString &basePath, bool *ok ) {
    QString path;
    *ok = false;

    if ( basePath.isNull() || basePath.isEmpty() || filename.isNull() || filename.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::buildPath: invalid path\n" ));
        return "";
    }

    path = basePath + filename;
    path.replace( '\\', FS_PATH_SEPARATOR );
    *ok = true;

    return path;
}

/*
============
fileExists
============
*/
bool Sys_Filesystem::fileExistsExt( QString &filename, int &flags, int &searchPathIndex ) {
    bool ok;
    int y = -1;

    if (( flags & FS_FLAGS_ABSOLUTE )) {
        QFile tmpFile( filename );
        if ( tmpFile.open( QIODevice::ReadOnly )) {
            tmpFile.close();
            return true;
        }
    } else {
        foreach ( searchPath_t *sp, this->searchPaths ) {
            y++;

            if ( sp->type == SEARCHPATH_DIRECTORY ) {
                if ( flags & FS_FLAGS_PACKSONLY )
                    continue;

                QFile tmpFile( this->buildPath( filename, sp->path, &ok ));
                if ( !ok ) {
                    y++;
                    continue;
                } else {
                    if ( !tmpFile.open( QIODevice::ReadOnly )) {
                        if ( fs_debug->integer())
                            com.print( this->tr( "^6Sys_Filesystem::fileExists: cannot open file \"%1\": %2\n" ).arg( tmpFile.fileName()).arg( tmpFile.errorString()));
                        continue;
                    }

                    searchPathIndex = y;
                    tmpFile.close();
                    return true;
                }
            } else if ( sp->type == SEARCHPATH_PACKAGE ) {
                if ( flags & FS_FLAGS_DIRSONLY )
                    continue;

                foreach ( packedFileInfo_t *pFile, sp->package->fileList ) {
                    if ( !QString::compare( pFile->name, filename, Qt::CaseInsensitive )) {
                        searchPathIndex = y;
                        return true;
                    }
                }
            }
        }
    }

#ifdef Q_OS_WIN
    // handle win32 lnks
    if ( !( flags & FS_FLAGS_LINKED ) && !fs_ignoreLinks->integer()) {
        QString linkedFilename;
        int linkedFlags;

        // copy info
        linkedFilename = filename + FS_LNK_EXT;
        linkedFlags = flags;
        linkedFlags |= FS_FLAGS_LINKED;

        // search for the link
        if ( this->fileExists( linkedFilename, linkedFlags )) {
            lnkInfo_t info;
            if ( this->readLink( linkedFilename, info, linkedFlags )) {
                if ( !info.isDirectory ) {
                    linkedFlags |= FS_FLAGS_ABSOLUTE;
                    linkedFilename = info.driveName + info.path;
                    if ( this->fileExists( linkedFilename, linkedFlags )) {
                        flags = linkedFlags;
                        filename = linkedFilename;
                        return true;
                    }
                }
            }
        }
    }
#endif
    return false;
}

/*
============
fOpenFileRead
============
*/
int Sys_Filesystem::fOpenFileRead( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, int flags ) {
    unz_t *pFileInfo;
    FILE  *pTemp;
    searchPath_t *sp;
    fsFileInfo_t *fsFile;
    Q_UNUSED( flags );

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read
    fsFile = new fsFileInfo_t();
    this->fileList << fsFile;
    fsFile->handle = fHandle;
    fsFile->openMode = FS_MODE_READ;
    fsFile->searchPathIndex = searchPathIndex;

    // absolute path
    if ( flags & FS_FLAGS_ABSOLUTE ) {
        fsFile->fHandle.setFileName( filename );
        if ( !fsFile->fHandle.open( QIODevice::ReadOnly ))
            return -1;

        fsFile->filename = filename;
        fsFile->type = SEARCHPATH_DIRECTORY;
        return fsFile->fHandle.size();
    } else
        sp = this->searchPaths.at( searchPathIndex );

    // open the file
    if ( sp->type == SEARCHPATH_PACKAGE ) {
        foreach ( packedFileInfo_t *pFile, sp->package->fileList ) {
            // case insensitive search for packages
            if ( !QString::compare( pFile->name, filename, Qt::CaseInsensitive )) {
                fsFile->pHandle = sp->package->handle;
                fsFile->type = SEARCHPATH_PACKAGE;
                fsFile->filename = filename;
                pFileInfo = ( unz_t* )fsFile->pHandle;

                // in case the file was new
                pTemp = pFileInfo->file;

                // set the file position in the package (also sets the current file info)
                pkg.setCurrentFileInfoPosition( sp->package->handle, pFile->pos );

                // copy the file info into the unzip structure
                memcpy( pFileInfo, sp->package->handle, sizeof( unz_t ));

                // we copy this back into the structure
                pFileInfo->file = pTemp;

                // open the file in the package
                pkg.openCurrentFile( fsFile->pHandle );
                fsFile->packagePos = pFile->pos;

                // return length
                return pFileInfo->cur_file_info.uncompressed_size;
            }
        }
    } else {
        fsFile->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path ));
        if ( !fsFile->fHandle.open( QIODevice::ReadOnly ))
            return -1;

        fsFile->filename = filename;
        fsFile->type = SEARCHPATH_DIRECTORY;
        return fsFile->fHandle.size();
    }
    return -1;
}

/*
============
fOpenFileWrite
============
*/
void Sys_Filesystem::fOpenFileWrite( const QString &filename, fileHandle_t &fHandle, int flags ) {
    fsFileInfo_t *fsFile;
    Q_UNUSED( flags );

    // absolute path
    if ( flags & FS_FLAGS_ABSOLUTE ) {
        com.error( ERR_FATAL, this->tr( "Sys_Filesystem::fOpenFileWrite: (FS_FLAGS_ABSOLUTE) writing to files outside searchPaths not supported\n" ));
        return;
    }

    // we store all new files at home
    int searchPathIndex = this->getSearchPathIndex( FS_HOMEPATH_ID );

    // failed
    if ( searchPathIndex < 0 )
        com.error( ERR_FATAL, this->tr( "Sys_Filesystem::fOpenFileWrite: could not resolve homeDir\n" ));

    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::fOpenFileWrite: open \"%1\" in \"%2\" in write mode\n" ).arg( filename, this->searchPaths.at( searchPathIndex )->path ));

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read in write mode
    fsFile = new fsFileInfo_t();
    this->fileList << fsFile;
    fsFile->openMode = FS_MODE_WRITE;
    fsFile->searchPathIndex = searchPathIndex;
    fsFile->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path ));
    QDir tmpDir( this->searchPaths.at( searchPathIndex )->path );
    if ( !tmpDir.exists()) {
        if ( !( flags & FS_FLAGS_SILENT ))
            com.print( this->tr( "^2Sys_Filesystem::fOpenFileWrite: ^3creating non-existant path \"%1\"\n" ).arg( this->searchPaths.at( searchPathIndex )->path ));
        tmpDir.mkpath( this->searchPaths.at( searchPathIndex )->path );

        if ( !tmpDir.exists()) {
            com.error( ERR_FATAL, this->tr( "Sys_Filesystem::fOpenFileWrite: could not create path \"%1\"\n" ).arg( this->searchPaths.at( searchPathIndex )->path ));
            return;
        }
    }

    if ( !fsFile->fHandle.open( QIODevice::WriteOnly )) {
        if ( !( flags & FS_FLAGS_SILENT ))
            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFileWrite: could not open \"%1\" in write mode\n" ).arg( filename ));
        return;
    }

    fsFile->handle = fHandle;
    fsFile->filename = filename;
    fsFile->type = SEARCHPATH_DIRECTORY;
}

/*
============
fOpenFileAppend
============
*/
int Sys_Filesystem::fOpenFileAppend( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, int flags ) {
    fsFileInfo_t *fsFile;
    Q_UNUSED( flags );

    // absolute path
    if ( flags & FS_FLAGS_ABSOLUTE ) {
        com.error( ERR_FATAL, this->tr( "Sys_Filesystem::fOpenFileAppend: (FS_FLAGS_ABSOLUTE) appending files outside searchPaths not supported\n" ));
        return -1;
    }

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read in append mode
    fsFile = new fsFileInfo_t();
    this->fileList << fsFile;
    fsFile->openMode = FS_MODE_APPEND;
    fsFile->searchPathIndex = searchPathIndex;
    fsFile->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path ));
    if ( !fsFile->fHandle.open( QIODevice::Append )) {
        if ( !( flags & FS_FLAGS_SILENT ))
            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFileAppend: could not open \"%1\" in append mode\n" ).arg( filename ));
        return -1;
    }

    fsFile->handle = fHandle;
    fsFile->filename = filename;
    fsFile->type = SEARCHPATH_DIRECTORY;

    return fsFile->fHandle.size();
}

/*
============
fOpenFile
============
*/
int Sys_Filesystem::fOpenFile( int mode, const QString &filename, fileHandle_t &fHandle, int flags ) {
    int fsp;
    QString path;

    // minus one means fail
    int fileLength = -1;

    // copy path
    path = filename;

    // failsafe
    if ( !this->initialized || this->searchPaths.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFile: filesystem not initialized\n" ));
        return fileLength;
    }

    // we cannot have these symbols in paths
    if ( path.contains( ".." ) || path.contains( "::" )) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFile: invalid path \"%1\"\n" ).arg( path ));
        return fileLength;
    }

    // check if it exists at all
    if ( !this->fileExists( path, flags, fsp ) && mode != FS_MODE_WRITE ) {
        if ( !( flags & FS_FLAGS_SILENT ))
            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFile: could not open file \"%1\"\n" ).arg( path ));
        return fileLength;
    } else {
        if ( mode != FS_MODE_WRITE ) {
            if ( fs_debug->integer()) {
                if ( !( flags & FS_FLAGS_ABSOLUTE )) {
                    if ( this->searchPaths.at( fsp )->type == SEARCHPATH_DIRECTORY )
                        com.print( this->tr( "^6Sys_Filesystem::fOpenFile: success for \"%1\" in \"%2\" (dir)\n" ).arg( path, this->searchPaths.at( fsp )->path ));
                    else
                        com.print( this->tr( "^6Sys_Filesystem::fOpenFile: success for \"%1\" in \"%2\" (package)\n" ).arg( path, this->searchPaths.at( fsp )->path ));
                } else {
                    com.print( this->tr( "^6Sys_Filesystem::fOpenFile: success for \"%1\" (absolute)\n" ).arg( path ));
                }
            }
        }

        // already open?
        if ( !( flags & FS_FLAGS_FORCE )) {
            foreach ( fsFileInfo_t *fsFile, this->fileList ) {
                if ( !QString::compare( path, fsFile->filename/*, Qt::CaseInsensitive */) && ( fsp == fsFile->searchPathIndex ) && fsFile->openMode >= 0 ) {
                    switch( fsFile->openMode ) {
                    case FS_MODE_READ:
                        if ( !( flags & FS_FLAGS_SILENT ))
                            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFile: file \"%1\" already open in read mode\n" ).arg( path ));
                        break;

                    case FS_MODE_APPEND:
                        if ( !( flags & FS_FLAGS_SILENT ))
                            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFile: file \"%1\" already open in append mode\n" ).arg( path ));
                        break;

                    case FS_MODE_WRITE:
                        if ( !( flags & FS_FLAGS_SILENT ))
                            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fOpenFile: file \"%1\" already open in write mode\n" ).arg( path ));
                        break;
                    }
                    return fileLength;
                }
            }
        }

        // open according to mode
        switch( mode ) {
        case FS_MODE_READ:
            fileLength = this->fOpenFileRead( path, fHandle, fsp, flags );
            break;

        case FS_MODE_WRITE:
            this->fOpenFileWrite( path, fHandle, flags );
            fileLength = 0;
            break;

        case FS_MODE_APPEND:
            fileLength = this->fOpenFileAppend( path, fHandle, fsp, flags );
            break;

        default:
            com.error( ERR_FATAL, this->tr( "Sys_FileSystem::fOpenFile: invalid mode %1\n" ).arg( mode ));
            return -1;
        }
    }
    return fileLength;
}

/*
============
fCloseFile
============
*/
void Sys_Filesystem::fCloseFile( const QString &filename, int flags ) {
    // failsafe
    if ( !this->initialized || this->searchPaths.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fCloseFile: filesystem not initialized\n" ));
        return;
    }

    foreach ( fsFileInfo_t *fsFile, this->fileList ) {
        if ( !QString::compare( filename, fsFile->filename )) {
            if ( fsFile->openMode == FS_MODE_CLOSED ) {
                if ( !( flags & FS_FLAGS_SILENT ))
                    com.print( this->tr( "^2Sys_Filesystem::fCloseFile: ^3file \"%1\" already closed\n" ).arg( filename ));
                return;
            }
            fsFile->openMode = FS_MODE_CLOSED;
            if ( fsFile->type == SEARCHPATH_DIRECTORY )
                fsFile->fHandle.close();
            else
                pkg.closeCurrentFile( fsFile->pHandle );

            return;
        }
    }
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::fCloseFile: file \"%1\" has not been opened\n" ).arg( filename ));
}

/*
============
fCloseFile
============
*/
void Sys_Filesystem::fCloseFile( const fileHandle_t fHandle, int flags ) {
    // failsafe
    if ( !this->initialized || this->searchPaths.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::fCloseFile: filesystem not initialized\n" ));
        return;
    }

    if ( !fHandle )
        return;

    foreach ( fsFileInfo_t *fsFile, this->fileList ) {
        if ( fHandle == fsFile->handle ) {
            if ( fsFile->openMode == FS_MODE_CLOSED ) {
                if ( !( flags & FS_FLAGS_SILENT ))
                    com.print( this->tr( "^2Sys_Filesystem::fCloseFile: ^3file \"%1\" already closed\n" ).arg( fsFile->filename ));
                return;
            }
            fsFile->openMode = FS_MODE_CLOSED;

            if ( fs_debug->integer())
                com.print( this->tr( "^6Sys_Filesystem::fCloseFile: closing \"%1\"\n" ).arg( fsFile->filename ));

            if ( fsFile->type == SEARCHPATH_DIRECTORY )
                fsFile->fHandle.close();
            else
                pkg.closeCurrentFile( fsFile->pHandle );

            return;
        }
    }
    com.print( this->tr( "^2Sys_Filesystem::fCloseFile: ^3file with handle %1 has not been opened\n" ).arg( fHandle ));
}

/*
============
read
============
*/
int Sys_Filesystem::read( byte *buffer, int len, fileHandle_t fHandle, int flags ) {
    // failsafe
    if ( !this->initialized || this->searchPaths.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::read: filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::read: called without fileHandle\n" ));
        return -1;
    }

    foreach ( fsFileInfo_t *fsFile, this->fileList ) {
        if ( fHandle == fsFile->handle ) {
            if ( fsFile->openMode != FS_MODE_READ ) {
                if ( !( flags & FS_FLAGS_SILENT ))
                    com.error( ERR_SOFT, this->tr( "Sys_Filesystem::read: file \"%1\" opened in wrong mode, aborting\n" ).arg( fsFile->filename ));
                return -1;
            }

            if ( fsFile->type == SEARCHPATH_DIRECTORY )
                return fsFile->fHandle.read( (char*)buffer, len );
            else
                return pkg.readCurrentFile( fsFile->pHandle, (void*)buffer, len );
        }
    }
    if ( !( flags & FS_FLAGS_SILENT ))
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::read: file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
write
============
*/
int Sys_Filesystem::write( const byte *buffer, int len, fileHandle_t fHandle, int flags ) {
    // failsafe
    if ( !this->initialized || this->searchPaths.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::write: filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::write: called without fileHandle\n" ));
        return -1;
    }

    foreach ( fsFileInfo_t *fsFile, this->fileList ) {
        if ( fHandle == fsFile->handle ) {
            if ( fsFile->openMode != FS_MODE_WRITE ) {
                if ( !( flags & FS_FLAGS_SILENT ))
                    com.error( ERR_SOFT, this->tr( "Sys_Filesystem::write: file \"%1\" opened in wrong mode\n" ).arg( fsFile->filename ));
                return -1;
            }

            if ( fsFile->type == SEARCHPATH_DIRECTORY )
                return fsFile->fHandle.write( QByteArray( (const char*)buffer, len ));
            else {
                if ( !( flags & FS_FLAGS_SILENT ))
                    com.error( ERR_SOFT, this->tr( "Sys_Filesystem::write: file \"%1\" is in a package\n" ).arg( fsFile->filename ));
                return -1;
            }
        }
    }
    if ( !( flags & FS_FLAGS_SILENT ))
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::write: file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
seek
============
*/
bool Sys_Filesystem::seek( fileHandle_t fHandle, long offset, int flags, int seekMode ) {
    // failsafe
    if ( !this->initialized || this->searchPaths.isEmpty()) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::seek: filesystem not initialized\n" ));
        return false;
    }

    if ( !fHandle ) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::seek: called without fileHandle\n" ));
        return false;
    }

    foreach ( fsFileInfo_t *fsFile, this->fileList ) {
        if ( fHandle == fsFile->handle ) {
            if ( fsFile->openMode != FS_MODE_READ ) {
                if ( !( flags & FS_FLAGS_SILENT ))
                    com.error( ERR_SOFT, this->tr( "Sys_Filesystem::seek: file \"%1\" is not open in read mode\n" ).arg( fsFile->filename ));
                return false;
            }

            if ( fsFile->type == SEARCHPATH_DIRECTORY ) {
                if ( seekMode == FS_SEEK_SET )
                    return fsFile->fHandle.seek( offset );
                else if ( seekMode == FS_SEEK_CURRENT )
                    return fsFile->fHandle.seek( fsFile->fHandle.pos() + offset );
                else if ( seekMode == FS_SEEK_END )
                    return fsFile->fHandle.seek( fsFile->fHandle.size());
                else {
                    com.error( ERR_FATAL, this->tr( "Sys_Filesystem::seek: unknown seek mode\n" ));
                    return false;
                }
            } else {
                byte buffer[FS_PACKAGE_SEEK_BUFFER];
                int remaining = offset;

                if( offset < 0 || seekMode == FS_SEEK_END ) {
                    com.error( ERR_FATAL, this->tr( "Sys_Filesystem::seek: negative offsets and FS_SEEK_END not supported for packaged files\n" ));
                    return -1;
                }

                switch( seekMode ) {
                case FS_SEEK_SET:
                    pkg.setCurrentFileInfoPosition( fsFile->pHandle, fsFile->packagePos );
                    pkg.openCurrentFile( fsFile->pHandle );
                    return true;

                case FS_SEEK_CURRENT:
                    while( remaining > FS_PACKAGE_SEEK_BUFFER ) {
                        this->read( buffer, FS_PACKAGE_SEEK_BUFFER, fHandle );
                        remaining -= FS_PACKAGE_SEEK_BUFFER;
                    }
                    this->read( buffer, remaining, fHandle );
                    return true;

                default:
                    com.error( ERR_FATAL, this->tr( "Sys_Filesystem::seek: unknown seek mode\n" ));
                    return false;
                }
            }
        }
    }
    if ( !( flags & FS_FLAGS_SILENT ))
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::seek: file with handle %1 has not been opened\n" ).arg( fHandle ));
    return false;
}

/*
=================
fPrint
=================
*/
void Sys_Filesystem::fPrint( const fileHandle_t fHandle, const QString &msg, int flags ) {
    this->write((const byte*)msg.toLatin1().constData(), msg.length(), fHandle, flags );
}

/*
============
readFile
============
*/
int Sys_Filesystem::readFile( const QString &filename, byte **buffer, int flags ) {
    int len;
    fileHandle_t fHandle;
    byte *buf = NULL;

    // look for it in the filesystem
    len = this->fOpenFile( FS_MODE_READ, filename, fHandle, flags );
    if ( len <= 0 ) {
        if ( !( flags & FS_FLAGS_SILENT ))
            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::readFile: could not read file \"%1\"\n" ).arg( filename ));
        return len;
    }

    if ( !buffer ) {
        if ( !( flags & FS_FLAGS_SILENT ))
            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::readFile: called with NULL buffer for \"%1\"\n" ).arg( filename ));
        this->fCloseFile( fHandle, flags );
        return len;
    }

    // allocate in memory and read
    buf = new byte[len+1];
    *buffer = buf;
    this->read( buf, len, fHandle, flags );

    // guarantee that it will have a trailing 0 for string operations
    buf[len] = 0;
    this->fCloseFile( fHandle, flags );

    // dd, add to buffer list
    this->fileBuffers << new YFileBuffer( filename, buf, fHandle );

    // return length
    return len;
}

/*
============
freeFile
============
*/
void Sys_Filesystem::freeFile( const QString &filename ) {
    foreach ( YFileBuffer *bPtr, this->fileBuffers ) {
        if ( !QString::compare( bPtr->filename, filename )) {
            if ( fs_debug->integer())
                com.print( this->tr( "^6Sys_Filesystem::freeFile: clearing buffer for file \"%1\"\n" ).arg( filename ));
            delete[] bPtr->buffer;
            this->fileBuffers.removeOne( bPtr );
        }
    }
}

/*
============
shutdown
============
*/
void Sys_Filesystem::shutdown() {
    // failsafe
    if ( !this->initialized )
        return;

    // announce
    com.print( this->tr( "^2Sys_Filesystem: ^5shutting down filesystem\n" ));

    // clear all file buffers still open
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::shutdown: clearing file buffers (%1)\n" ).arg( this->fileBuffers.count()));

    foreach ( YFileBuffer *bPtr, this->fileBuffers )
        delete[] bPtr->buffer;
    this->fileBuffers.clear();

    // delete searchPath content
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::shutdown: clearing searchPaths (%1)\n" ).arg( this->searchPaths.count()));

    foreach ( searchPath_t *sp, this->searchPaths ) {
        if ( sp->type == SEARCHPATH_PACKAGE ) {
            foreach ( packedFileInfo_t *pFile, sp->package->fileList )
                delete pFile;

            delete sp->package;
        }
        delete sp;
    }
    this->searchPaths.clear();

    // close all open files and clear the list
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::shutdown: clearing files (%1)\n" ).arg( this->fileList.count()));

    foreach ( fsFileInfo_t *fsFile, this->fileList ) {
        if ( fsFile->openMode >= 0 ) {
            if ( fsFile->type == SEARCHPATH_DIRECTORY )
                fsFile->fHandle.close();
            else
                pkg.closeCurrentFile( fsFile->pHandle );

        }
        delete fsFile;
    }
    this->fileList.clear();

    // remove commmands
    cmd.removeCommand( "fs_touch" );
    cmd.removeCommand( "fs_list" );
}

/*
============
touch
============
*/
void Sys_Filesystem::touch( const QString &filename, int flags ) {
    int handle;
    fs.fOpenFileWrite( filename, handle, flags );
    fs.fCloseFile( handle, flags );
}

/*
============
touch
============
*/
void Sys_Filesystem::touch() {
    if ( cmd.argc() < 2 ) {
        com.print( this->tr( "^3usage: fs_touch [filename]\n" ));
        return;
    }
    this->touch( cmd.argv(1));
}

/*
============
list
============
*/
QStringList Sys_Filesystem::list( const QString &directory, const QRegExp &filter, int mode ) {
    bool ok;
    QDir dir;
    QStringList foundFiles;
    QString path;
    QString searchDir;
    int curDepth = 0;
    int entryDepth;

    searchDir = directory;
    if ( !searchDir.endsWith( "/" ))
        searchDir.append( "/" );

    path = this->buildPath( searchDir, fs_homePath->string() + fs_baseDir->string(), &ok );
    if ( ok ) {
        // this is how we get local files
        dir = QDir( path );

        if ( mode == FS_LIST_ALL )
            dir.setFilter( QDir::NoDotAndDotDot | QDir::AllEntries );
        else if ( mode == FS_LIST_FILES_ONLY )
            dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
        else if ( mode == FS_LIST_DIRS_ONLY )
            dir.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
        else {
            com.error( ERR_SOFT, this->tr( "Sys_Filesystem::list: invalid list mode\n" ));
            return QStringList();
        }
        QFileInfoList entryList = dir.entryInfoList();

        // make sure to prepend the dir
        foreach ( QFileInfo info, entryList ) {
            if ( info.isDir()) {
                if ( directory == "." )
                    foundFiles << info.fileName().append( "/" );
                else
                    foundFiles << info.fileName().append( "/" ).prepend( searchDir );
            } else {
                if ( directory != "." )
                    foundFiles << info.fileName().prepend( searchDir );
                else
                    foundFiles << info.fileName();
            }
        }

        foreach ( searchPath_t *sp, this->searchPaths ) {
            // this is how we get the packaged ones
            if ( sp->type == SEARCHPATH_PACKAGE ) {
                foreach ( packedFileInfo_t *pFilePtr, sp->package->fileList ) {
                    if ( directory == "." || pFilePtr->name.startsWith( searchDir )) {
                        // filter subdirs
                        if ( directory != "." )
                            curDepth = searchDir.count( "/" );

                        if ( pFilePtr->name.endsWith( "/" ))
                            entryDepth = pFilePtr->name.count( "/" ) - 1;
                        else
                            entryDepth = pFilePtr->name.count( "/" );

                        if ( entryDepth == curDepth ) {
                            if ( mode == FS_LIST_ALL )
                                foundFiles << pFilePtr->name;
                            else if ( mode == FS_LIST_FILES_ONLY ) {
                                if ( !pFilePtr->name.endsWith( "/" ))
                                    foundFiles << pFilePtr->name;
                            } else if ( mode == FS_LIST_DIRS_ONLY ) {
                                if ( pFilePtr->name.endsWith( "/" ))
                                    foundFiles << pFilePtr->name;
                            } else {
                                com.error( ERR_SOFT, this->tr( "Sys_Filesystem::list: invalid list mode\n" ));
                                return QStringList();
                            }
                        }
                    }
                }
            }
        }
        // filter
        foundFiles = foundFiles.filter( filter );
        foundFiles.removeDuplicates();

        // return our files
        return foundFiles;
    } else
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::list: invalid path\n" ));

    // nothing
    return QStringList();
}

/*
============
list
============
*/
void Sys_Filesystem::list() {
    if ( cmd.argc() < 2 ) {
        com.print( this->tr( "^3usage: fs_list [directory] (filter)\n" ));
        return;
    }

    QStringList filteredList;

    if ( cmd.argc() == 3 )
        filteredList = this->list( cmd.argv(1), QRegExp( cmd.argv(2)));
    else
        filteredList = this->list( cmd.argv(1));

    // announce
    com.print( this->tr( "^3Directory \"%1\"\n" ).arg( cmd.argv(1)));

    // print out
    foreach ( QString str, filteredList )
        com.print( QString( "^2 %1\n" ).arg( str ));
}

/*
============
defaultExtenstion
============
*/
void Sys_Filesystem::defaultExtension( QString &filename, const QString &extension ) {
    if ( !filename.endsWith( extension ))
        filename.append( extension );
}

/*
================
extractFromPackage

 used for modules, ported form ET-GPL code
 using home directory as priority
================
*/
bool Sys_Filesystem::extractFromPackage( const QString &filename ) {
    int srcLength;
    int destLength;
    byte *srcData;
    byte *destData;
    bool needToCopy;
    FILE *destHandle;
    needToCopy = true;
    QString homePath;
    bool ok;

    // read in compressed file
    srcLength = fs.readFile( filename, &srcData, FS_FLAGS_PACKSONLY );

    // if its not in the package, we bail
    if ( srcLength == -1 )
        return false;

    // read in local file in homeDir
    homePath = this->buildPath( filename, fs_homePath->string() + fs_baseDir->string(), &ok );
    if ( !ok ) {
        com.error( ERR_SOFT, this->tr( "Sys_Filesystem::extractFromPackage: could not resolve homePath\n" ));
        return false;
    }

    homePath.replace( "/", QDir::separator());
    destHandle = fopen( homePath.toLatin1().constData(), "rb" );

    // if we have a local file, we need to compare the two
    if ( destHandle ) {
        fseek( destHandle, 0, SEEK_END );
        destLength = ftell( destHandle );
        fseek( destHandle, 0, SEEK_SET );

        if ( destLength > 0 ) {
            // allocate
            destData = new byte[destLength];

            // and read
            size_t size = fread( destData, destLength, 1, destHandle );
            Q_UNUSED( size );

            // compare files
            if ( destLength == srcLength ) {
                int y;

                for ( y = 0; y < destLength; y++ )
                    if ( destData[y] != srcData[y] )
                        break;

                if ( y == destLength )
                    needToCopy = false;
            }

            // clean up
            delete[] destData;
        }

        fclose( destHandle );
    }

    // write file
    if ( needToCopy ) {
        com.print( this->tr( "^3Sys_FileSystem::extractFromPackage: \"^5%1^3\" mismatch or is missing in homeDir, copying from package\n" ).arg( filename ));
        fileHandle_t f;

        this->fOpenFile( FS_MODE_WRITE, filename, f );
        if ( !f ) {
            com.error( ERR_SOFT, this->tr( "Sys_FileSystem::extractFromPackage: failed to open \"%1\"\n" ).arg( filename ));
            return false;
        }

        this->write( srcData, srcLength, f );
        this->fCloseFile( f );
    }

    // clear buffer
    fs.freeFile( filename );
    return true;
}

/*
================
readLink

 handle win32 lnks as pseudo-symlinks (experimental)
 win32 API is idiotic, so this is a port from GPL'd kfile_plugins
================
*/
#ifdef Q_OS_WIN32
bool Sys_Filesystem::readLink( const QString &filename, lnkInfo_t &info, int flags ) {
    lnkHeader_t header;
    fileHandle_t fHandle;

    if ( !( flags & FS_FLAGS_SILENT ))
        com.print( this->tr( "^3Sys_FileSystem::readLink: reading win32 link \"%1\"\n" ).arg( filename.mid( filename.lastIndexOf( "\\" ) + 1 )));

    if ( this->fOpenFile( FS_MODE_READ, filename, fHandle, flags ) <= 0 ) {
        com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read win32 link \"%1\"\n" ).arg( filename ));
        return false;
    }

    if ( !this->read(( byte* )&header, sizeof( header ), fHandle )) {
        com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read header of win32 link \"%1\"\n" ).arg( filename ));
        this->fCloseFile( fHandle );
        return false;
    }

    if ( memcmp( header.magic, "L\0\0\0", 4 ) != 0 ) {
        com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: incorrect header magic for win32 link \"%1\"\n" ).arg( filename ));
        this->fCloseFile( fHandle );
        return false;
    }

    // the shell item id list is present
    if ( header.flags & 0x1 ) {
        quint16 len;

        // skip that list
        if ( !this->read(( byte* )&len, sizeof( len ), fHandle ) || ( !this->seek( fHandle, len, flags, 1 ))) {
            com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read shell item id list for win32 link \"%1\"\n" ).arg( filename ));
            this->fCloseFile( fHandle );
            return false;
        }
    }

    info.isDirectory = ( header.attributes & 0x10 );

    // not a directory
    if ( !info.isDirectory )
        info.fileSize = header.length;

    // points to file or directory
    info.isFileOrDir = ( header.flags & 0x2 );

    if ( info.isFileOrDir ) {
        lnkFileLocation_t loc;

        if ( !this->read(( byte* )&loc, sizeof( loc ), fHandle )) {
            com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read shell item id list for win32 link \"%1\"\n" ).arg( filename ));
            this->fCloseFile( fHandle );
            return false;
        }

        // limit the following "new", because the size to allocate is in the file
        // which can easily be manipulted to contain a huge number and lead to a crash
        // 4096 is just an arbitrary number I think shall be enough
        if (( loc.totalLen <= sizeof( loc )) || ( loc.totalLen > 4096 )) {
            this->fCloseFile( fHandle );
            return false;
        }

        size_t size = loc.totalLen - sizeof( loc );
        char *data = new char[size];
        char *start = data - sizeof( loc );

        if ( !this->read(( byte* )data, size, fHandle )) {
            com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read path data for win32 link \"%1\"\n" ).arg( filename ));
            this->fCloseFile( fHandle );
            return false;
        }

        info.isNetworkPath = !( loc.flags & 0x1 );

        if ( !info.isNetworkPath ) {
            // volume label
            info.volumeName = ( start + loc.localVolume + 0x10 );
            info.path = QString::null;

            if ( *( start + loc.basePath )) {
                // don't put any more than "X:" into info.driveName
                info.driveName = *( start + loc.basePath );
                info.driveName += ':';

                // if we in fact do have more than just "X:", store any additional
                // path information separately in info.path
                if ( *( start + loc.basePath + 1 ) == ':' &&
                        *( start + loc.basePath + 2 ) != 0 )
                    info.path = ( start + loc.basePath + 2 );
            }

            if ( *( start + loc.pathname ) != 0 ) {
                if ( info.path.isNull() )
                    info.path = ( start + loc.pathname );
                else
                    info.path = info.path + "\\" + ( start + loc.pathname );
            }
            // network path
            // dd, do we really need to handle network path?
        } else {
            info.path = QString( "%1\\%2" )
                    .arg( start + loc.netVolume + 0x14 )  // network share name
                    .arg( start + loc.pathname );
        }

        delete [] data;
        data = 0;

        // has description string
        if ( header.flags & 0x4 ) {
            quint16 len;

            if ( !this->read(( byte* )&len, sizeof( len ), fHandle )) {
                com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read description string length for win32 link \"%1\"\n" ).arg( filename ));
                this->fCloseFile( fHandle );
                return false;
            }

            // this can never be > 65K, so its OK to not check the size
            data = new char[len+1];

            if ( !this->read(( byte* )data, len, fHandle )) {
                com.error( ERR_SOFT, this->tr( "Sys_FileSystem::readLink: could not read description string for win32 link \"%1\"\n" ).arg( filename ));
                delete [] data;
                this->fCloseFile( fHandle );
                return false;
            }

            // nullbyte seems to miss
            data[len] = 0;
            info.description = data;
            delete [] data;
        }
    }

    this->fCloseFile( fHandle );
    return true;
}
#endif
