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
class Sys_Filesystem fs;

//
// cvars
//
pCvar *fs_homePath;
pCvar *fs_appPath;
pCvar *fs_debug;
pCvar *fs_basePath;
pCvar *fs_resolveLinks;

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
    // get homePath
    QString homePath = QDir::homePath();
    homePath.append( Filesystem::PathSeparator );
    homePath.append( Filesystem::PlatformDirectory );

    // create cvars
    fs_homePath = cv.create( "fs_homePath", homePath.replace( '\\', Filesystem::PathSeparator ).append( Filesystem::PathSeparator ), ( pCvar::ReadOnly ));
    fs_appPath = cv.create( "fs_appPath", QDir::currentPath().replace( '\\', Filesystem::PathSeparator ).append( Filesystem::PathSeparator ), ( pCvar::ReadOnly ));
    fs_basePath = cv.create( "fs_basePath", Filesystem::BaseDirectory + Filesystem::PathSeparator, ( pCvar::Flags )( pCvar::Archive | pCvar::ReadOnly ));
    fs_debug = cv.create( "fs_debug", false, pCvar::Archive );
    fs_resolveLinks = cv.create( "fs_resolveLinks", true, pCvar::Archive );

    // add search paths
#ifndef YP2_FINAL_RELEASE
    // dev dir takes priority
    this->addSearchPath( QDir( "../yplatform2" ).absolutePath().append( "/" ).append( fs_basePath->string()), Filesystem::ProjectPathID );
#endif
    this->addSearchPath( ":/", Filesystem::InternalPathID );
    this->addSearchPath( fs_homePath->string().append( fs_basePath->string()), Filesystem::HomePathID );
    this->addSearchPath( fs_appPath->string().append( fs_basePath->string()), Filesystem::AppPathID );

    // reset handles
    // -1 and 0 is reserved
    this->numFileHandles = 1;

    // add commmands
    cmd.add( "fs_touch", touchCmd, this->tr( "create an empty file" ));
    cmd.add( "fs_list", listCmd, this->tr( "list contents of a directory" ));

    // all done
    this->setInitialized();

    // load packages only after base initialization is complete
    this->loadPackages();

    // for debugging
    if ( fs_debug->isEnabled()) {
        foreach ( pSearchPath *sp, this->searchPaths ) {
            if ( sp->type() == pFile::Directory )
                com.print( StrDebug + this->tr( "%1 (dir)\n" ).arg( sp->path()));
            else
                com.print( StrDebug + this->tr( "searchPath: %1 (package)\n" ).arg( sp->path()));

        }
    }

    // add resources
    QDir::setSearchPaths( ":", QStringList( ":/" ));
}


/*
============
shutdown
============
*/
void Sys_Filesystem::shutdown() {
    // failsafe
    if ( !this->hasInitialized())
        return;

    // announce
    com.print( StrMsg + this->tr( "shutting down filesystem\n" ));

    // delete searchPath content
    if ( fs_debug->isEnabled())
        com.print( StrDebug + this->tr( "clearing searchPaths (%1)\n" ).arg( this->searchPaths.count()));

    foreach ( pSearchPath *sp, this->searchPaths )
        delete sp;
    this->searchPaths.clear();

    // close all open files and clear the list
    if ( fs_debug->isEnabled())
        com.print( StrDebug + this->tr( "clearing files (%1)\n" ).arg( this->fileList.count()));

    foreach ( pFile *filePtr, this->fileList ) {
        if ( filePtr->mode() >= pFile::Read ) {
            if ( filePtr->pathType() == pFile::Directory )
                filePtr->fHandle.close();
            else
                filePtr->pHandle->close();
        }
        delete filePtr;
    }
    this->fileList.clear();

    // remove commmands
    cmd.remove( "fs_touch" );
    cmd.remove( "fs_list" );
}

/*
============
loadPackages
============
*/
void Sys_Filesystem::loadPackages() {
    int y = 0;
    bool dup;

    // find packages in searchpaths
    foreach ( pSearchPath *sp, this->searchPaths ) {
        if ( sp->type() == pFile::Directory ) {
            QDir dir( sp->path());
            dir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
            QStringList filters;
            filters << QString( "*" ).append( Filesystem::DefaultPackageExtension );
            dir.setNameFilters( filters );
            dir.setSorting( QDir::Name | QDir::Reversed );

            // get pFile::Package list
            QFileInfoList list = dir.entryInfoList();

            foreach ( QFileInfo fInfo, list ) {
                dup = false;

                // ignore duplicate packages
                foreach ( pSearchPath *sp, this->searchPaths ) {
                    if ( !QString::compare( fInfo.fileName(), sp->path())) {
                        dup = true;
                        break;                    }
                }
                if ( dup )
                    continue;

                // load the package
                pPackage *pPtr = pkg.load( fInfo.fileName(), Sys_Filesystem::Silent );
                if ( pPtr != NULL )
                    this->addSearchPath( pPtr, fInfo.fileName());
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
void Sys_Filesystem::addSearchPath( const QString &path, const QString &id ) {
    pSearchPath *searchPath = new pSearchPath();
    searchPath->setPath( path );
    searchPath->setId( id );
    searchPath->setType( pFile::Directory );
    this->searchPaths << searchPath;
}

/*
============
addSearchPath
============
*/
void Sys_Filesystem::addSearchPath( pPackage *package, const QString &filename, const QString &id ) {
    pSearchPath *searchPath = new pSearchPath();
    searchPath->setPackage( package );
    searchPath->setPath( filename );
    searchPath->setId( id );
    searchPath->setType( pFile::Package );
    this->searchPaths << searchPath;
}

/*
============
getSearchPathIndex
============
*/
int Sys_Filesystem::getSearchPathIndex( const QString &id ) const {
    int y = 0;

    foreach ( pSearchPath *sp, this->searchPaths ) {
        if ( !QString::compare( id, sp->id()))
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
        com.error( StrSoftError + this->tr( "invalid path\n" ));
        return "";
    }

    path = basePath + filename;
    path.replace( '\\', Filesystem::PathSeparator );
    *ok = true;

    return path;
}

/*
============
existsExt
============
*/
bool Sys_Filesystem::existsExt( QString &filename, OpenFlags &flags, int &searchPathIndex ) {
    bool ok;
    int y = -1;

    if ( filename.startsWith( ":/" )) {
        // abort with skipInternal flag and :/ paths
        if ( flags.testFlag( Sys_Filesystem::SkipInternal ))
            return false;

        if ( flags.testFlag( Absolute )) {
            com.error( StrFatalError + this->tr( "(Absolute) flag set for internal path\n" ));
            return false;
        }

        if ( QFile::exists( filename )) {
            searchPathIndex = this->getSearchPathIndex( Filesystem::InternalPathID );
            return true;
        } else {
            return false;
        }
    }

    if ( flags.testFlag( Absolute )) {
        QFile tmpFile( filename );
        if ( tmpFile.open( QIODevice::ReadOnly )) {
            tmpFile.close();
            return true;
        }
    } else {
        foreach ( pSearchPath *sp, this->searchPaths ) {
            y++;

            // skip internal paths
            if ( sp->id() == Filesystem::InternalPathID && flags.testFlag( Sys_Filesystem::SkipInternal ))
                continue;

            if ( sp->type() == pFile::Directory ) {
                if ( flags.testFlag( PacksOnly ))
                    continue;

                QFile tmpFile( this->buildPath( filename, sp->path(), &ok ));
                if ( !ok ) {
                    y++;
                    continue;
                } else {
                    if ( !tmpFile.open( QIODevice::ReadOnly )) {
                        if ( fs_debug->isEnabled())
                            com.print( StrDebug + this->tr( "cannot open file \"%1\": %2\n" ).arg( tmpFile.fileName()).arg( tmpFile.errorString()));
                        continue;
                    }

                    searchPathIndex = y;
                    tmpFile.close();
                    return true;
                }
            } else if ( sp->type() == pFile::Package ) {
                if ( flags.testFlag( DirsOnly ))
                    continue;

                foreach ( pEntry *pEntry, sp->package()->fileList ) {
                    if ( !QString::compare( pEntry->name(), filename, Qt::CaseInsensitive )) {
                        searchPathIndex = y;
                        return true;
                    }
                }
            }
        }
    }

#ifdef Q_OS_WIN
    // handle win32 lnks
    if ( !( flags.testFlag( Linked )) && fs_resolveLinks->isEnabled()) {
        QString linkedFilename;
        OpenFlags linkedFlags;

        // copy info
        linkedFilename = filename + Filesystem::DefaultLinkExtension;
        linkedFlags = flags;
        linkedFlags |= Linked;

        // search for the link
        if ( this->exists( linkedFilename, linkedFlags )) {
            lnkInfo_t info;
            if ( this->readLink( linkedFilename, info, linkedFlags )) {
                if ( !info.isDirectory ) {
                    linkedFlags |= Absolute;
                    linkedFilename = info.driveName + info.path;
                    if ( this->exists( linkedFilename, linkedFlags )) {
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
openInReadMode
============
*/
int Sys_Filesystem::openInReadMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags ) {
    pSearchPath *sp;
    pFile *filePtr;

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read
    filePtr = new pFile();
    this->fileList << filePtr;
    filePtr->setHandle( fHandle );
    filePtr->setOpenMode( pFile::Read );
    filePtr->setPathIndex( searchPathIndex );

    // absolute path
    if ( flags.testFlag( Absolute )) {
        filePtr->fHandle.setFileName( filename );
        if ( !filePtr->fHandle.open( QIODevice::ReadOnly ))
            return -1;

        filePtr->setName( filename );
        filePtr->setPathType( pFile::Directory );
        return filePtr->fHandle.size();
    } else
        sp = this->searchPaths.at( searchPathIndex );

    // open the file
    if ( sp->type() == pFile::Package ) {
        pEntry *pEntryPtr = pkg.find( filename );
        if ( pEntryPtr != NULL ) {
            filePtr->pHandle = pEntryPtr;
            filePtr->setPathType( pFile::Package );
            filePtr->setName( pEntryPtr->name());
            return pEntryPtr->open();
        }
    } else {
        if ( !filename.startsWith( ":/" ))
            filePtr->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path()));
        else
            filePtr->fHandle.setFileName( filename );

        if ( !filePtr->fHandle.open( QIODevice::ReadOnly ))
            return -1;

        filePtr->setName( filename );
        filePtr->setPathType( pFile::Directory );
        return filePtr->fHandle.size();
    }
    return -1;
}

/*
============
openInWriteMode
============
*/
void Sys_Filesystem::openInWriteMode( const QString &filename, fileHandle_t &fHandle, OpenFlags flags ) {
    pFile *filePtr;

    // failsafe
    if ( filename.startsWith( ":/" )) {
        com.error( StrFatalError + this->tr( "cannot write to internal filesystem\n" ));
        return;
    }

    // failsafe
    if ( filename.endsWith( Filesystem::DefaultPackageExtension )) {
        com.error( StrFatalError + this->tr( "openInWriteMode: cannot write to package\n" ));
        return;
    }

    // we store all new files at home
    int searchPathIndex = this->getSearchPathIndex( Filesystem::HomePathID );

    // failed
    if ( searchPathIndex < 0 ) {
        com.error( StrFatalError + this->tr( "openInWriteMode: could not resolve homePath\n" ));
        return;
    }

    if ( fs_debug->isEnabled())
        com.print( StrDebug + this->tr( "open \"%1\" in \"%2\" in write mode\n" ).arg( filename, this->searchPaths.at( searchPathIndex )->path()));

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read in write mode
    filePtr = new pFile();
    this->fileList << filePtr;
    filePtr->setOpenMode( pFile::Write );
    filePtr->setPathIndex( searchPathIndex );

    QDir tmpDir;
    if ( flags.testFlag( Absolute )) {
        filePtr->fHandle.setFileName( filename );
        tmpDir = QFileInfo( filePtr->fHandle ).absoluteDir();
    } else {
        filePtr->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path()));
        tmpDir.setPath( this->searchPaths.at( searchPathIndex )->path() + QFileInfo( filename ).path());
    }

    if ( !tmpDir.exists()) {
        if ( !( flags.testFlag( Silent )))
            com.print( StrDebug + this->tr( "creating non-existant path \"%1\"\n" ).arg( tmpDir.absolutePath()));

        tmpDir.mkpath( tmpDir.absolutePath());
        if ( !tmpDir.exists()) {
            com.error( StrFatalError + this->tr( "could not create path \"%1\"\n" ).arg( this->searchPaths.at( searchPathIndex )->path()));
            return;
        }
    }

    if ( !filePtr->fHandle.open( QIODevice::WriteOnly )) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not open \"%1\" in write mode\n" ).arg( filename ));
        return;
    }

    filePtr->setHandle( fHandle );
    filePtr->setName( filename );
    filePtr->setPathType( pFile::Directory );
}

/*
============
openInAppendMode
============
*/
int Sys_Filesystem::openInAppendMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags ) {
    pFile *filePtr;

    // absolute path
    if ( flags.testFlag( Absolute )) {
        com.error( StrFatalError + this->tr( "(Absolute) appending files outside searchPaths not supported\n" ));
        return -1;
    }

    // failsafe
    if ( filename.startsWith( ":/" )) {
        com.error( StrFatalError + this->tr( "cannot append to internal filesystem files\n" ));
        return -1;
    }

    // failsafe
    if ( filename.endsWith( Filesystem::DefaultPackageExtension )) {
        com.error( StrFatalError + this->tr( "cannot write to package\n" ));
        return -1;
    }

    // assign next available handle
    fHandle = this->numFileHandles++;

    // allocate and mark as currently being read in append mode
    filePtr = new pFile();
    this->fileList << filePtr;
    filePtr->setOpenMode( pFile::Append );
    filePtr->setPathIndex( searchPathIndex );
    filePtr->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path()));
    if ( !filePtr->fHandle.open( QIODevice::Append )) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not open \"%1\" in append mode\n" ).arg( filename ));
        return -1;
    }

    filePtr->setHandle( fHandle );
    filePtr->setName( filename );
    filePtr->setPathType( pFile::Directory );

    return filePtr->fHandle.size();
}

/*
============
open
============
*/
int Sys_Filesystem::open( pFile::OpenModes mode, const QString &filename, fileHandle_t &fHandle, OpenFlags flags ) {
    int fsp;
    QString path;

    // minus one means fail
    int fileLength = -1;

    // copy path
    path = filename;

    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return fileLength;
    }

    // abort with skipInternal flag and :/ paths
    if ( filename.startsWith( ":/" ) && flags.testFlag( Sys_Filesystem::SkipInternal ))
        return fileLength;

    // we cannot have these symbols in paths
    if ( path.contains( ".." ) || path.contains( "::" )) {
        com.error( StrSoftError + this->tr( "invalid path \"%1\"\n" ).arg( path ));
        return fileLength;
    }

    // check if it exists at all
    if ( !this->exists( path, flags, fsp ) && mode != pFile::Write ) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not open file \"%1\"\n" ).arg( path ));
        return fileLength;
    } else {
        if ( mode != pFile::Write ) {
            if ( fs_debug->isEnabled()) {
                if ( !( flags.testFlag( Absolute ))) {
                    if ( this->searchPaths.at( fsp )->type() == pFile::Directory )
                        com.print( StrDebug + this->tr( "success for \"%1\" in \"%2\" (Dir)\n" ).arg( path, this->searchPaths.at( fsp )->path()));
                    else
                        com.print( StrDebug + this->tr( "success for \"%1\" in \"%2\" (Package)\n" ).arg( path, this->searchPaths.at( fsp )->path()));
                } else {
                    com.print( StrDebug + this->tr( "success for \"%1\" (Absolute)\n" ).arg( path ));
                }
            }
        }

        // already open?
        if ( !( flags.testFlag( Force ))) {
            foreach ( pFile *filePtr, this->fileList ) {
                if ( !QString::compare( path, filePtr->name()/*, Qt::CaseInsensitive */) && ( fsp == filePtr->pathIndex()) && filePtr->mode() >= pFile::Read ) {
                    switch( filePtr->mode()) {
                    case pFile::Closed:
                        break;

                    case pFile::Read:
                        if ( !( flags.testFlag( Silent )))
                            com.error( StrSoftError + this->tr( "file \"%1\" already open in read mode\n" ).arg( path ));
                        break;

                    case pFile::Append:
                        if ( !( flags.testFlag( Silent )))
                            com.error( StrSoftError + this->tr( "file \"%1\" already open in append mode\n" ).arg( path ));
                        break;

                    case pFile::Write:
                        if ( !( flags.testFlag( Silent )))
                            com.error( StrSoftError + this->tr( "file \"%1\" already open in write mode\n" ).arg( path ));
                        break;
                    }
                    return fileLength;
                }
            }
        }

        // open according to mode
        switch( mode ) {
        case pFile::Read:
            fileLength = this->openInReadMode( path, fHandle, fsp, flags );
            break;

        case pFile::Write:
            this->openInWriteMode( path, fHandle, flags );
            fileLength = 0;
            break;

        case pFile::Append:
            fileLength = this->openInAppendMode( path, fHandle, fsp, flags );
            break;

        default:
            com.error( StrFatalError + this->tr( "invalid mode %1\n" ).arg( mode ));
            return -1;
        }
    }
    return fileLength;
}

/*
============
close
============
*/
void Sys_Filesystem::close( const QString &filename, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( !QString::compare( filename, filePtr->name())) {
            if ( filePtr->mode() == pFile::Closed ) {
                if ( !( flags.testFlag( Silent )))
                    com.print( StrWarn + this->tr( "file \"%1\" already closed\n" ).arg( filename ));
                return;
            }
            filePtr->setOpenMode( pFile::Closed );
            if ( filePtr->pathType() == pFile::Directory )
                filePtr->fHandle.close();
            else
                filePtr->pHandle->close();

            return;
        }
    }
    if ( fs_debug->isEnabled())
        com.print( StrDebug + this->tr( "file \"%1\" has not been opened\n" ).arg( filename ));
}

/*
============
close
============
*/
void Sys_Filesystem::close( const fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return;
    }

    if ( !fHandle )
        return;

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() == pFile::Closed ) {
                if ( !( flags.testFlag( Silent )))
                    com.print( StrWarn + this->tr( "file \"%1\" already closed\n" ).arg( filePtr->name()));
                return;
            }
            filePtr->setOpenMode( pFile::Closed );

            if ( fs_debug->isEnabled())
                com.print( StrDebug + this->tr( "closing \"%1\"\n" ).arg( filePtr->name()));

            if ( filePtr->pathType() == pFile::Directory )
                filePtr->fHandle.close();
            else
                filePtr->pHandle->close();

            return;
        }
    }
    com.print( StrWarn + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
}

/*
============
read
============
*/
int Sys_Filesystem::read( byte *buffer, unsigned int len, fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( StrSoftError + this->tr( "called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Read ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" opened in wrong mode, aborting\n" ).arg( filePtr->name()));
                return -1;
            }

            if ( filePtr->pathType() == pFile::Directory )
                return filePtr->fHandle.read( reinterpret_cast<char*>( buffer ), len );
            else
                return filePtr->pHandle->read( buffer, len, flags );
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( StrSoftError + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
write
============
*/
int Sys_Filesystem::write( const byte *buffer, unsigned int len, fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( StrSoftError + this->tr( "called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Write ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" opened in wrong mode\n" ).arg( filePtr->name()));
                return -1;
            }

            if ( filePtr->pathType() == pFile::Directory )
                return filePtr->fHandle.write( QByteArray( reinterpret_cast<const char*>( buffer ), len ));
            else {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" is in a package\n" ).arg( filePtr->name()));
                return -1;
            }
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( StrSoftError + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
write
============
*/
int Sys_Filesystem::write( const QByteArray buffer, fileHandle_t fHandle, OpenFlags flags ) {
    return this->write( reinterpret_cast<const byte*>( buffer.constData()), buffer.length(), fHandle, flags );
}

/*
============
seek
============
*/
bool Sys_Filesystem::seek( fileHandle_t fHandle, int offset, OpenFlags flags, SeekModes seekMode ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return false;
    }

    if ( !fHandle ) {
        com.error( StrSoftError + this->tr( "called without fileHandle\n" ));
        return false;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Read ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( StrSoftError + this->tr( "file \"%1\" is not open in read mode\n" ).arg( filePtr->name()));
                return false;
            }

            if ( filePtr->pathType() == pFile::Directory ) {
                if ( seekMode == Set )
                    return filePtr->fHandle.seek( offset );
                else if ( seekMode == Current )
                    return filePtr->fHandle.seek( filePtr->fHandle.pos() + offset );
                else if ( seekMode == End )
                    return filePtr->fHandle.seek( filePtr->fHandle.size());
                else {
                    com.error( StrFatalError + this->tr( "unknown seek mode\n" ));
                    return false;
                }
            } else {
                // for now
                if ( offset < 0 || seekMode == End ) {
                    com.error( StrFatalError + this->tr( "cannot seek in packages\n" ));
                    return false;
                }
                byte buffer[Filesystem::PackageSeekBuffer];
                int remaining = offset;

                if( offset < 0 || seekMode == End ) {
                    com.error( StrFatalError + this->tr( "negative offsets and (End) mode not supported for packaged files\n" ));
                    return false;
                }

                switch( seekMode ) {
                case Set:
                    // reset file
                    filePtr->pHandle->open();
                    remaining = offset;

                    // perform read until offset
                    while ( remaining ) {
                        if ( static_cast<unsigned int>( offset ) >= Filesystem::PackageSeekBuffer ) {
                            remaining -= Filesystem::PackageSeekBuffer;
                            this->read( buffer, Filesystem::PackageSeekBuffer, fHandle, flags );
                            continue;
                        }
                        this->read( buffer, remaining, fHandle, flags );
                        break;
                    }
                    return true;

                case Current:
                    while ( static_cast<unsigned int>( remaining ) > Filesystem::PackageSeekBuffer ) {
                        this->read( buffer, Filesystem::PackageSeekBuffer, fHandle );
                        remaining -= Filesystem::PackageSeekBuffer;
                    }
                    this->read( buffer, remaining, fHandle );
                    return true;

                default:
                    com.error( StrFatalError + this->tr( "unknown seek mode\n" ));
                    return false;
                }
            }
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( StrSoftError + this->tr( "file with handle %1 has not been opened\n" ).arg( fHandle ));
    return false;
}

/*
=================
print
=================
*/
void Sys_Filesystem::print( const fileHandle_t fHandle, const QString &msg, OpenFlags flags ) {
    this->write( reinterpret_cast<const byte*>( msg.toLatin1().constData()), msg.length(), fHandle, flags );
}

/*
============
readFile
============
*/
QByteArray Sys_Filesystem::readFile( const QString &filename, OpenFlags flags ) {
    int len;
    fileHandle_t fHandle;
    byte *buf = NULL;
    QByteArray buffer;

    // abort with skipInternal flag and :/ paths
    if ( filename.startsWith( ":/" ) && flags.testFlag( Sys_Filesystem::SkipInternal ))
        return QByteArray();

    // look for it in the filesystem
    len = this->open( pFile::Read, filename, fHandle, flags );

    if ( len <= 0 ) {
        if ( !( flags.testFlag( Silent )))
            com.error( StrSoftError + this->tr( "could not read file \"%1\"\n" ).arg( filename ));
        return QByteArray();
    }

    // allocate in memory, read and store as a byte array
    buf = new byte[len];
    this->read( buf, len, fHandle, flags );
    buffer = QByteArray( reinterpret_cast<const char*>( buf ), len );

    // clear temporary buffer, close file
    delete []buf;
    this->close( fHandle, flags );

    // return buffer
    return buffer;
}

/*
============
touch
============
*/
void Sys_Filesystem::touch( const QString &filename, OpenFlags flags ) {
    fileHandle_t handle;
    this->openInWriteMode( filename, handle, flags );
    this->close( handle, flags );
}

/*
============
touch
============
*/
void Sys_Filesystem::touch( const QStringList &args ) {
    if ( args.count() != 1 ) {
        com.print( Sys::cYellow + this->tr( "usage: ^2fs_touch ^3[^2filename^3]\n" ));
        return;
    }
    this->touch( args.first());
}

/*
============
listDirectory
============
*/
QStringList Sys_Filesystem::listDirectory( QString searchDir, const QString &path, ListModes mode ) {
    QStringList foundFiles;
    QDir dir;

    // this is how we get local files
    if ( searchDir != path )
        dir = QDir( path + searchDir );
    else
        dir = QDir( path );

    if ( mode == ListAll )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::AllEntries );
    else if ( mode == ListFiles )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
    else if ( mode == ListDirs )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    else {
        com.error( StrSoftError + this->tr( "invalid list mode\n" ));
        return QStringList();
    }
    QFileInfoList entryList = dir.entryInfoList();
    foreach ( QFileInfo info, entryList ) {
        if ( searchDir.startsWith( "/" )) {
            if ( !info.isDir())
                foundFiles << searchDir.remove( 0, 1 ) + info.fileName();
            else
                foundFiles << searchDir.remove( 0, 1 ) + info.fileName().append( "/" );
        } else {
            if ( !info.isDir())
                foundFiles << searchDir + info.fileName();
            else
                foundFiles << searchDir + info.fileName().append( "/" );
        }
    }
    return foundFiles;
}

/*
============
list
============
*/
QStringList Sys_Filesystem::list( QString searchDir, const QRegExp &filter, ListModes mode ) {
    QStringList foundFiles;
    int curDepth = 0;
    int entryDepth;

    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( StrSoftError + this->tr( "filesystem not initialized\n" ));
        return QStringList();
    }

    if ( searchDir.startsWith( "." )) {
        com.error( StrSoftError + this->tr( "Dot and DotDot paths not supported\n" ));
        return QStringList();
    }

    if ( !searchDir.endsWith( "/" ))
        searchDir.append( "/" );

    foreach ( pSearchPath *sp, this->searchPaths ) {
        // this is how we get the packaged ones
        if ( sp->type() == pFile::Package ) {
            foreach ( pEntry *pFilePtr, sp->package()->fileList ) {
                if ( pFilePtr->name().startsWith( searchDir ) || searchDir == "/" ) {
                    // filter subdirs
                    if ( searchDir != "/" )
                        curDepth = searchDir.count( "/" );

                    if ( pFilePtr->name().endsWith( "/" ))
                        entryDepth = pFilePtr->name().count( "/" ) - 1;
                    else
                        entryDepth = pFilePtr->name().count( "/" );

                    if ( entryDepth == curDepth ) {
                        if ( mode == ListAll )
                            foundFiles << pFilePtr->name();
                        else if ( mode == ListFiles ) {
                            if ( !pFilePtr->name().endsWith( "/" ))
                                foundFiles << pFilePtr->name();
                        } else if ( mode == ListDirs ) {
                            if ( pFilePtr->name().endsWith( "/" ))
                                foundFiles << pFilePtr->name();
                        } else {
                            com.error( StrSoftError + this->tr( "invalid list mode\n" ));
                            return QStringList();
                        }
                    }
                }
            }
        } else if ( sp->type() == pFile::Directory ) {
            // assuming nobody tampered with the list priority goes like this:
            //   internal > home > app > (project)
            foundFiles << this->listDirectory( searchDir, sp->path(), mode );
        }
    }

    // filter
    foundFiles = foundFiles.filter( filter );
    foundFiles.removeDuplicates();

    // return our files
    return foundFiles;
}

/*
============
list
============
*/
void Sys_Filesystem::list( const QStringList &args ) {
    QStringList filteredList;

    // in case of no args, just list base (root) directory
    if ( args.count() == 2 )
        filteredList = this->list( args.first(), QRegExp( args.at( 1 )));
    else if ( args.count() == 1 )
        filteredList = this->list( args.first());
    else
        filteredList = this->list( "/" );

    // announce
    if ( args.isEmpty())
        com.print( Sys::cYellow + this->tr( "Base directory\n" ));
    else
        com.print( Sys::cYellow + this->tr( "Directory of \"%1\"\n" ).arg( args.first()));

    // print out
    foreach ( QString str, filteredList )
        com.print( Sys::cYellow + QString( " %1\n" ).arg( str ));
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
============
defaultExtenstion
============
*/
QString Sys_Filesystem::defaultExtension( const QString &filename, const QString &extension ) const {
    if ( !filename.endsWith( extension ))
        return filename + extension;
    else
        return filename;
}

/*
================
extract
================
*/
bool Sys_Filesystem::extract( const QString &filename ) {
    QByteArray pkgBuffer, localBuffer;
    QString localFilename;
    bool ok;

    // read in compressed file
    pkgBuffer = this->readFile( filename, PacksOnly );

    // if it's not in the package, we bail
    if ( pkgBuffer.isNull()) {
        com.error( StrSoftError + this->tr( "could not open packaged file\n" ));
        return false;
    }

    // determine local path
    localFilename = this->buildPath( filename, fs_homePath->string() + fs_basePath->string(), &ok );
    if ( !ok ) {
        com.error( StrSoftError + this->tr( "could not resolve homePath\n" ));
        return false;
    }
    localFilename.replace( "/", QDir::separator());

    // open local file
    localBuffer = this->readFile( localFilename, ( Absolute | Silent ));

    // compare the two
    if ( localBuffer != pkgBuffer ) {
        fileHandle_t f;

        // files differ - copy over the packaged one
        this->open( pFile::Write, localFilename, f, Absolute );
        this->write( pkgBuffer, f );
        this->close( f );
    }
    
    localBuffer.clear();
    pkgBuffer.clear();
    return true;
}

/*
================
readLink

 handle win32 lnks as pseudo-symlinks (experimental)
 win32 API is idiotic, so this is a port from GPL'd kfile_plugins
================
*/
#ifdef Q_OS_WIN
bool Sys_Filesystem::readLink( const QString &filename, lnkInfo_t &info, OpenFlags flags ) {
    lnkHeader_t header;
    fileHandle_t fHandle;

#if 0
    // this message is annoying
    if ( !( flags.testFlag( Silent )))
        com.print( StrWarn + this->tr( "reading win32 link \"%1\"\n" ).arg( filename.mid( filename.lastIndexOf( "\\" ) + 1 )));
#endif

    if ( this->open( pFile::Read, filename, fHandle, flags ) <= 0 ) {
        com.error( StrSoftError + this->tr( "could not read win32 link \"%1\"\n" ).arg( filename ));
        return false;
    }

    if ( !this->read( reinterpret_cast<byte*>( &header ), sizeof( header ), fHandle )) {
        com.error( StrSoftError + this->tr( "could not read header of win32 link \"%1\"\n" ).arg( filename ));
        this->close( fHandle );
        return false;
    }

    if ( memcmp( header.magic, "L\0\0\0", 4 ) != 0 ) {
        com.error( StrSoftError + this->tr( "incorrect header magic for win32 link \"%1\"\n" ).arg( filename ));
        this->close( fHandle );
        return false;
    }

    // the shell item id list is present
    if ( header.flags & 0x1 ) {
        quint16 len;

        // skip that list
        if ( !this->read( reinterpret_cast<byte*>( &len ), sizeof( len ), fHandle ) || ( !this->seek( fHandle, len, flags, Current ))) {
            com.error( StrSoftError + this->tr( "could not read shell item id list for win32 link \"%1\"\n" ).arg( filename ));
            this->close( fHandle );
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

        if ( !this->read( reinterpret_cast<byte*>( &loc ), sizeof( loc ), fHandle )) {
            com.error( StrSoftError + this->tr( "could not read shell item id list for win32 link \"%1\"\n" ).arg( filename ));
            this->close( fHandle );
            return false;
        }

        // limit the following "new", because the size to allocate is in the file
        // which can easily be manipulted to contain a huge number and lead to a crash
        // 4096 is just an arbitrary number I think shall be enough
        if (( loc.totalLen <= sizeof( loc )) || ( loc.totalLen > 4096 )) {
            this->close( fHandle );
            return false;
        }

        size_t size = loc.totalLen - sizeof( loc );
        byte *data = new byte[size];
        char *start = reinterpret_cast<char*>( data ) - sizeof( loc );

        if ( !this->read( data, static_cast<unsigned int>( size ), fHandle )) {
            com.error( StrSoftError + this->tr( "could not read path data for win32 link \"%1\"\n" ).arg( filename ));
            this->close( fHandle );
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
        } else
            info.path = QString( "%1\\%2" ).arg( start + loc.netVolume + 0x14 ).arg( start + loc.pathname );

        delete [] data;
        data = 0;

        // has description string
        if ( header.flags & 0x4 ) {
            quint16 len;

            if ( !this->read( reinterpret_cast<byte*>( &len ), sizeof( len ), fHandle )) {
                com.error( StrSoftError + this->tr( "could not read description string length for win32 link \"%1\"\n" ).arg( filename ));
                this->close( fHandle );
                return false;
            }

            // this can never be > 65K, so its OK to not check the size
            data = new byte[len+1];

            if ( !this->read( data, len, fHandle )) {
                com.error( StrSoftError + this->tr( "could not read description string for win32 link \"%1\"\n" ).arg( filename ));
                delete [] data;
                this->close( fHandle );
                return false;
            }

            // nullbyte seems to miss
            data[len] = 0;
            info.description = reinterpret_cast<char*>( data );
            delete [] data;
        }
    }

    this->close( fHandle );
    return true;
}
#endif

/*
================
length
================
*/
int Sys_Filesystem::length( fileHandle_t handle ) const {
    const pFile *filePtr = this->fileForHandle( handle );
    if ( filePtr != NULL )
        return filePtr->fHandle.size();
    else
        return -1;
}

/*
================
infoForHandle
================
*/
pFile *Sys_Filesystem::fileForHandle( fileHandle_t handle ) const {
    foreach ( pFile *filePtr, this->fileList ) {
        if ( filePtr->handle() == handle )
            return filePtr;
    }
    return NULL;
}

