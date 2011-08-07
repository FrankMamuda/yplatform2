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
class Sys_Filesystem fs;

//
// cvars
//
pCvar *fs_homePath;
pCvar *fs_appPath;
pCvar *fs_debug;
pCvar *fs_basePath;
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
    // get homePath
    QString homePath = QDir::homePath();
    homePath.append( Filesystem::PathSeparator );
    homePath.append( Filesystem::PlatformDirectory );

    // create cvars
    fs_homePath = cv.create( "fs_homePath", homePath.replace( '\\', Filesystem::PathSeparator ).append( Filesystem::PathSeparator ), ( pCvar::ReadOnly ));
    fs_appPath = cv.create( "fs_appPath", QDir::currentPath().replace( '\\', Filesystem::PathSeparator ).append( Filesystem::PathSeparator ), ( pCvar::ReadOnly ));
    fs_basePath = cv.create( "fs_basePath", Filesystem::BaseDirectory + Filesystem::PathSeparator, ( pCvar::Flags )( pCvar::Archive | pCvar::ReadOnly ));
    fs_debug = cv.create( "fs_debug", "0", pCvar::Archive );
    fs_ignoreLinks = cv.create( "fs_ignoreLinks", "0", pCvar::Archive );

    // add searchPaths
    this->addSearchPath( fs_homePath->string().append( fs_basePath->string()), Filesystem::HomePathID );
    this->addSearchPath( fs_appPath->string().append( fs_basePath->string()), Filesystem::AppPathID );
    this->addSearchPath( ":/", Filesystem::InternalPathID );

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
    if ( fs_debug->integer()) {
        foreach ( pSearchPath *sp, this->searchPaths ) {
            if ( sp->type() == pFile::Directory )
                com.print( this->tr( "^6Sys_Filesystem::init: searchPath: %1 (dir)\n" ).arg( sp->path()));
            else
                com.print( this->tr( "^6Sys_Filesystem::init: searchPath: %1 (package)\n" ).arg( sp->path()));

        }
    }

    // add resources
    QDir::setSearchPaths( ":", QStringList(":/"));
}

/*
============
loadPackages
============
*/
void Sys_Filesystem::loadPackages() {
    int y = 0;

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
void Sys_Filesystem::addSearchPath( const QString &path, const QString id ) {
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
void Sys_Filesystem::addSearchPath( pPackage *package, const QString &filename, const QString id ) {
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
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::buildPath: invalid path\n" ));
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
        if ( flags.testFlag( Absolute )) {
            com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::existsExt: (Absolute) set for internal path\n" ));
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

            if ( sp->type() == pFile::Directory ) {
                if ( flags.testFlag( PacksOnly ))
                    continue;

                QFile tmpFile( this->buildPath( filename, sp->path(), &ok ));
                if ( !ok ) {
                    y++;
                    continue;
                } else {
                    if ( !tmpFile.open( QIODevice::ReadOnly )) {
                        if ( fs_debug->integer())
                            com.print( this->tr( "^6Sys_Filesystem::existsExt: cannot open file \"%1\": %2\n" ).arg( tmpFile.fileName()).arg( tmpFile.errorString()));
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
    if ( !( flags.testFlag( Linked )) && !fs_ignoreLinks->integer()) {
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
long Sys_Filesystem::openInReadMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags ) {
    pSearchPath *sp;
    pFile *filePtr;
    Q_UNUSED( flags );

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
    Q_UNUSED( flags );

    // failsafe
    if ( filename.startsWith( ":/" )) {
        com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInWriteMode: cannot write to internal filesystem\n" ));
        return;
    }

    // failsafe
    if ( filename.endsWith( Filesystem::DefaultPackageExtension )) {
        com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInWriteMode: cannot write to package\n" ));
        return;
    }

    // we store all new files at home
    int searchPathIndex = this->getSearchPathIndex( Filesystem::HomePathID );

    // failed
    if ( searchPathIndex < 0 )
        com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInWriteMode: could not resolve homePath\n" ));

    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::openInWriteMode: open \"%1\" in \"%2\" in write mode\n" ).arg( filename, this->searchPaths.at( searchPathIndex )->path()));

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
        tmpDir.setPath( QDir( filename ).dirName());
    } else {
        filePtr->fHandle.setFileName( QString( filename ).prepend( this->searchPaths.at( searchPathIndex )->path()));
        tmpDir.setPath( this->searchPaths.at( searchPathIndex )->path());
    }
    if ( !tmpDir.exists()) {
        if ( !( flags.testFlag( Silent )))
            com.print( this->tr( "^2Sys_Filesystem::openInWriteMode: ^3creating non-existant path \"%1\"\n" ).arg( this->searchPaths.at( searchPathIndex )->path()));

        if ( flags.testFlag( Absolute ))
            tmpDir.mkpath( QDir( filename ).dirName());
        else
            tmpDir.mkpath( this->searchPaths.at( searchPathIndex )->path());

        if ( !tmpDir.exists()) {
            com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInWriteMode: could not create path \"%1\"\n" ).arg( this->searchPaths.at( searchPathIndex )->path()));
            return;
        }
    }

    if ( !filePtr->fHandle.open( QIODevice::WriteOnly )) {
        if ( !( flags.testFlag( Silent )))
            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::openInWriteMode: could not open \"%1\" in write mode\n" ).arg( filename ));
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
long Sys_Filesystem::openInAppendMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags ) {
    pFile *filePtr;
    Q_UNUSED( flags );

    // absolute path
    if ( flags.testFlag( Absolute )) {
        com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInAppendMode: (Absolute) appending files outside searchPaths not supported\n" ));
        return -1;
    }

    // failsafe
    if ( filename.startsWith( ":/" )) {
        com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInAppendMode: cannot append internal filesystem files\n" ));
        return -1;
    }

    // failsafe
    if ( filename.endsWith( Filesystem::DefaultPackageExtension )) {
        com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::openInAppendMode: cannot write to package\n" ));
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
            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::openInAppendMode: could not open \"%1\" in append mode\n" ).arg( filename ));
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
long Sys_Filesystem::open( pFile::OpenModes mode, const QString &filename, fileHandle_t &fHandle, OpenFlags flags ) {
    int fsp;
    QString path;

    // minus one means fail
    long fileLength = -1;

    // copy path
    path = filename;

    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::open: filesystem not initialized\n" ));
        return fileLength;
    }

    // we cannot have these symbols in paths
    if ( path.contains( ".." ) || path.contains( "::" )) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::open: invalid path \"%1\"\n" ).arg( path ));
        return fileLength;
    }

    // check if it exists at all
    if ( !this->exists( path, flags, fsp ) && mode != pFile::Write ) {
        if ( !( flags.testFlag( Silent )))
            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::open: could not open file \"%1\"\n" ).arg( path ));
        return fileLength;
    } else {
        if ( mode != pFile::Write ) {
            if ( fs_debug->integer()) {
                if ( !( flags.testFlag( Absolute ))) {
                    if ( this->searchPaths.at( fsp )->type() == pFile::Directory )
                        com.print( this->tr( "^6Sys_Filesystem::open: success for \"%1\" in \"%2\" (dir)\n" ).arg( path, this->searchPaths.at( fsp )->path()));
                    else
                        com.print( this->tr( "^6Sys_Filesystem::open: success for \"%1\" in \"%2\" (package)\n" ).arg( path, this->searchPaths.at( fsp )->path()));
                } else {
                    com.print( this->tr( "^6Sys_Filesystem::open: success for \"%1\" (absolute)\n" ).arg( path ));
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
                            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::open: file \"%1\" already open in read mode\n" ).arg( path ));
                        break;

                    case pFile::Append:
                        if ( !( flags.testFlag( Silent )))
                            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::open: file \"%1\" already open in append mode\n" ).arg( path ));
                        break;

                    case pFile::Write:
                        if ( !( flags.testFlag( Silent )))
                            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::open: file \"%1\" already open in write mode\n" ).arg( path ));
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
            com.error( Sys_Common::FatalError, this->tr( "Sys_FileSystem::open: invalid mode %1\n" ).arg( mode ));
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
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::close: filesystem not initialized\n" ));
        return;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( !QString::compare( filename, filePtr->name())) {
            if ( filePtr->mode() == pFile::Closed ) {
                if ( !( flags.testFlag( Silent )))
                    com.print( this->tr( "^2Sys_Filesystem::close: ^3file \"%1\" already closed\n" ).arg( filename ));
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
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::close: file \"%1\" has not been opened\n" ).arg( filename ));
}

/*
============
close
============
*/
void Sys_Filesystem::close( const fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::close: filesystem not initialized\n" ));
        return;
    }

    if ( !fHandle )
        return;

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() == pFile::Closed ) {
                if ( !( flags.testFlag( Silent )))
                    com.print( this->tr( "^2Sys_Filesystem::close: ^3file \"%1\" already closed\n" ).arg( filePtr->name()));
                return;
            }
            filePtr->setOpenMode( pFile::Closed );

            if ( fs_debug->integer())
                com.print( this->tr( "^6Sys_Filesystem::close: closing \"%1\"\n" ).arg( filePtr->name()));

            if ( filePtr->pathType() == pFile::Directory )
                filePtr->fHandle.close();
            else
                filePtr->pHandle->close();

            return;
        }
    }
    com.print( this->tr( "^2Sys_Filesystem::close: ^3file with handle %1 has not been opened\n" ).arg( fHandle ));
}

/*
============
read
============
*/
long Sys_Filesystem::read( byte *buffer, unsigned long len, fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::read: filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::read: called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Read ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::read: file \"%1\" opened in wrong mode, aborting\n" ).arg( filePtr->name()));
                return -1;
            }

            if ( filePtr->pathType() == pFile::Directory )
                return filePtr->fHandle.read((char*)buffer, len );
            else
                return filePtr->pHandle->read( buffer, len, flags );
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::read: file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
write
============
*/
long Sys_Filesystem::write( const byte *buffer, unsigned long len, fileHandle_t fHandle, OpenFlags flags ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::write: filesystem not initialized\n" ));
        return -1;
    }

    if ( !fHandle ) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::write: called without fileHandle\n" ));
        return -1;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Write ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::write: file \"%1\" opened in wrong mode\n" ).arg( filePtr->name()));
                return -1;
            }

            if ( filePtr->pathType() == pFile::Directory )
                return filePtr->fHandle.write( QByteArray((const char*)buffer, len ));
            else {
                if ( !( flags.testFlag( Silent )))
                    com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::write: file \"%1\" is in a package\n" ).arg( filePtr->name()));
                return -1;
            }
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::write: file with handle %1 has not been opened\n" ).arg( fHandle ));
    return -1;
}

/*
============
seek
============
*/
bool Sys_Filesystem::seek( fileHandle_t fHandle, long offset, OpenFlags flags, SeekModes seekMode ) {
    // failsafe
    if ( !this->hasInitialized() || this->searchPaths.isEmpty()) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::seek: filesystem not initialized\n" ));
        return false;
    }

    if ( !fHandle ) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::seek: called without fileHandle\n" ));
        return false;
    }

    foreach ( pFile *filePtr, this->fileList ) {
        if ( fHandle == filePtr->handle()) {
            if ( filePtr->mode() != pFile::Read ) {
                if ( !( flags.testFlag( Silent )))
                    com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::seek: file \"%1\" is not open in read mode\n" ).arg( filePtr->name()));
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
                    com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::seek: unknown seek mode\n" ));
                    return false;
                }
            } else {
                // for now
                if ( offset < 0 || seekMode == End ) {
                    com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::seek: cannot seek in packages\n" ));
                    return false;
                }
                byte buffer[Filesystem::PackageSeekBuffer];
                long remaining = offset;

                if( offset < 0 || seekMode == End ) {
                    com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::seek: negative offsets and End not supported for packaged files\n" ));
                    return false;
                }

                switch( seekMode ) {
                case Set:
                    // reset file
                    filePtr->pHandle->open();
                    remaining = offset;

                    // perform read until offset
                    while ( remaining ) {
                        if ((unsigned)offset >= Filesystem::PackageSeekBuffer ) {
                            remaining -= Filesystem::PackageSeekBuffer;
                            fs.read( buffer, Filesystem::PackageSeekBuffer, fHandle, flags );
                            continue;
                        }
                        fs.read( buffer, remaining, fHandle, flags );
                        break;
                    }
                    return true;

                case Current:
                    while ((unsigned)remaining > Filesystem::PackageSeekBuffer ) {
                        this->read( buffer, Filesystem::PackageSeekBuffer, fHandle );
                        remaining -= Filesystem::PackageSeekBuffer;
                    }
                    this->read( buffer, remaining, fHandle );
                    return true;

                default:
                    com.error( Sys_Common::FatalError, this->tr( "Sys_Filesystem::seek: unknown seek mode\n" ));
                    return false;
                }
            }
        }
    }
    if ( !( flags.testFlag( Silent )))
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::seek: file with handle %1 has not been opened\n" ).arg( fHandle ));
    return false;
}

/*
=================
print
=================
*/
void Sys_Filesystem::print( const fileHandle_t fHandle, const QString &msg, OpenFlags flags ) {
    this->write((const byte*)msg.toLatin1().constData(), msg.length(), fHandle, flags );
}

/*
============
readFile
============
*/
long Sys_Filesystem::readFile( const QString &filename, byte **buffer, OpenFlags flags ) {
    long len;
    fileHandle_t fHandle;
    byte *buf = NULL;

    // look for it in the filesystem
    len = this->open( pFile::Read, filename, fHandle, flags );

    if ( len <= 0 ) {
        if ( !( flags.testFlag( Silent )))
            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::readFile: could not read file \"%1\"\n" ).arg( filename ));
        return len;
    }

    if ( !buffer ) {
        if ( !( flags.testFlag( Silent )))
            com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::readFile: called with NULL buffer for \"%1\"\n" ).arg( filename ));
        this->close( fHandle, flags );
        return len;
    }

    // allocate in memory and read
    buf = new byte[len+1];
    *buffer = buf;
    this->read( buf, len, fHandle, flags );

    // guarantee that it will have a trailing 0 for string operations
    buf[len] = 0;
    this->close( fHandle, flags );

    // dd, add to buffer list
    this->fileBuffers << new pFileBuffer( filename, buf, fHandle );

    // return length
    return len;
}

/*
============
freeFile
============
*/
void Sys_Filesystem::freeFile( const QString &filename ) {
    foreach ( pFileBuffer *bPtr, this->fileBuffers ) {
        if ( !QString::compare( bPtr->filename(), filename )) {
            if ( fs_debug->integer())
                com.print( this->tr( "^6Sys_Filesystem::freeFile: clearing buffer for file \"%1\"\n" ).arg( filename ));
            bPtr->deleteBuffer();
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
    if ( !this->hasInitialized())
        return;

    // announce
    com.print( this->tr( "^2Sys_Filesystem: ^5shutting down filesystem\n" ));

    // clear all file buffers still open
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::shutdown: clearing file buffers (%1)\n" ).arg( this->fileBuffers.count()));

    foreach ( pFileBuffer *bPtr, this->fileBuffers )
        bPtr->deleteBuffer();
    this->fileBuffers.clear();

    // delete searchPath content
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::shutdown: clearing searchPaths (%1)\n" ).arg( this->searchPaths.count()));

    foreach ( pSearchPath *sp, this->searchPaths )
        delete sp;
    this->searchPaths.clear();

    // close all open files and clear the list
    if ( fs_debug->integer())
        com.print( this->tr( "^6Sys_Filesystem::shutdown: clearing files (%1)\n" ).arg( this->fileList.count()));

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
touch
============
*/
void Sys_Filesystem::touch( const QString &filename, OpenFlags flags ) {
    int handle;
    fs.openInWriteMode( filename, handle, flags );
    fs.close( handle, flags );
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
listDirectory
============
*/
QStringList Sys_Filesystem::listDirectory( const QString &searchDir, const QString &path, ListModes mode ) {
    QStringList foundFiles;
    QDir dir;

    // this is how we get local files
    dir = QDir( path );
    if ( mode == ListAll )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::AllEntries );
    else if ( mode == ListFiles )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
    else if ( mode == ListDirs )
        dir.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    else {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::listDirectory: invalid list mode\n" ));
        return QStringList();
    }
    QFileInfoList entryList = dir.entryInfoList();

    // make sure to prepend the dir
    foreach ( QFileInfo info, entryList ) {
        if ( info.isDir()) {
            if ( searchDir == "." )
                foundFiles << info.fileName().append( "/" );
            else {
                if ( path.startsWith( ":/" ))
                    foundFiles << info.fileName().prepend( "/" ).prepend( searchDir ).replace( "//", "/" );
                else
                    foundFiles << info.fileName().append( "/" ).prepend( searchDir );
            }
        } else {
            if ( searchDir != "." ) {
                if ( path.startsWith( ":/" ))
                    foundFiles << info.fileName().prepend( "/" ).prepend( searchDir ).replace( "//", "/" );
                else
                    foundFiles << info.fileName().prepend( searchDir );
            } else
                foundFiles << info.fileName();
        }
    }
    return foundFiles;
}

/*
============
list
============
*/
QStringList Sys_Filesystem::list( const QString &directory, const QRegExp &filter, ListModes mode ) {
    bool ok;
    QStringList foundFiles;
    QString path;
    QString searchDir;
    int curDepth = 0;
    int entryDepth;

    searchDir = directory;
    if ( !searchDir.endsWith( "/" ))
        searchDir.append( "/" );

    // internal files take priority
    if ( directory.startsWith( ":/" ))
        foundFiles << this->listDirectory( directory, directory, mode );

    // build fs path
    path = this->buildPath( searchDir, fs_homePath->string() + fs_basePath->string(), &ok );
    if ( ok ) {
        foundFiles << this->listDirectory( searchDir, path, mode );

        foreach ( pSearchPath *sp, this->searchPaths ) {
            // this is how we get the packaged ones
            if ( sp->type() == pFile::Package ) {
                foreach ( pEntry *pFilePtr, sp->package()->fileList ) {
                    if ( directory == "." || pFilePtr->name().startsWith( searchDir )) {
                        // filter subdirs
                        if ( directory != "." )
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
                                com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::list: invalid list mode\n" ));
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
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::list: invalid path\n" ));

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
extract

 used for modules, ported form ET-GPL code
 using home pFile::Directory as priority
================
*/
bool Sys_Filesystem::extract( const QString &filename ) {
    long srcLength;
    long destLength;
    byte *srcData;
    byte *destData;
    bool needToCopy;
    FILE *destHandle;
    needToCopy = true;
    QString homePath;
    bool ok;

    // read in compressed file
    srcLength = fs.readFile( filename, &srcData, PacksOnly );

    // if its not in the package, we bail
    if ( srcLength == -1 )
        return false;

    // read in local file in homePath
    homePath = this->buildPath( filename, fs_homePath->string() + fs_basePath->string(), &ok );
    if ( !ok ) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_Filesystem::extract: could not resolve homePath\n" ));
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
        com.print( this->tr( "^3Sys_FileSystem::extract: \"^5%1^3\" mismatch or is missing in homePath, copying from package\n" ).arg( filename ));
        fileHandle_t f;

        this->open( pFile::Write, filename, f );
        if ( !f ) {
            com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::extract: failed to open \"%1\"\n" ).arg( filename ));
            return false;
        }

        this->write( srcData, srcLength, f );
        this->close( f );
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
#ifdef Q_OS_WIN
bool Sys_Filesystem::readLink( const QString &filename, lnkInfo_t &info, OpenFlags flags ) {
    lnkHeader_t header;
    fileHandle_t fHandle;

    if ( !( flags.testFlag( Silent )))
        com.print( this->tr( "^3Sys_FileSystem::readLink: reading win32 link \"%1\"\n" ).arg( filename.mid( filename.lastIndexOf( "\\" ) + 1 )));

    if ( this->open( pFile::Read, filename, fHandle, flags ) <= 0 ) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read win32 link \"%1\"\n" ).arg( filename ));
        return false;
    }

    if ( !this->read(( byte* )&header, sizeof( header ), fHandle )) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read header of win32 link \"%1\"\n" ).arg( filename ));
        this->close( fHandle );
        return false;
    }

    if ( memcmp( header.magic, "L\0\0\0", 4 ) != 0 ) {
        com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: incorrect header magic for win32 link \"%1\"\n" ).arg( filename ));
        this->close( fHandle );
        return false;
    }

    // the shell item id list is present
    if ( header.flags & 0x1 ) {
        quint16 len;

        // skip that list
        if ( !this->read(( byte* )&len, sizeof( len ), fHandle ) || ( !this->seek( fHandle, len, flags, Current ))) {
            com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read shell item id list for win32 link \"%1\"\n" ).arg( filename ));
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

        if ( !this->read(( byte* )&loc, sizeof( loc ), fHandle )) {
            com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read shell item id list for win32 link \"%1\"\n" ).arg( filename ));
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
        char *data = new char[size];
        char *start = data - sizeof( loc );

        if ( !this->read(( byte* )data, size, fHandle )) {
            com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read path data for win32 link \"%1\"\n" ).arg( filename ));
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
                com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read description string length for win32 link \"%1\"\n" ).arg( filename ));
                this->close( fHandle );
                return false;
            }

            // this can never be > 65K, so its OK to not check the size
            data = new char[len+1];

            if ( !this->read(( byte* )data, len, fHandle )) {
                com.error( Sys_Common::SoftError, this->tr( "Sys_FileSystem::readLink: could not read description string for win32 link \"%1\"\n" ).arg( filename ));
                delete [] data;
                this->close( fHandle );
                return false;
            }

            // nullbyte seems to miss
            data[len] = 0;
            info.description = data;
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
long Sys_Filesystem::length( fileHandle_t handle ) const {
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

