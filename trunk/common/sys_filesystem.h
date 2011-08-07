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

#ifndef SYS_FILESYSTEM_H
#define SYS_FILESYSTEM_H

//
// includes
//
#include "sys_shared.h"
#ifndef MODULE_LIBRARY
#include "sys_package.h"
#endif
#ifdef Q_OS_WIN
#include "sys_link.h"
#endif

//
// classes
//
class pPackage;
class pEntry;

//
// namespaces
//
namespace Filesystem {
    class Sys_Filesystem;
    static const unsigned int MaxPackagePath = 256;
    static const QString DefaultPackageExtension( ".package" );
#ifdef Q_OS_WIN
    static const QString DefaultLinkExtension( ".lnk" );
#endif

    // we exclusively use forward slashes in fs, since QFile also supports these
    static const QChar PathSeparator( '/' );

    // basedir
    static const QString BaseDirectory( "base" );
#ifndef MODULE_LIBRARY
    static const unsigned int PackageSeekBuffer = 65536;

    // searchPath IDs for easier handling
    static const QString HomePathID( "homePath" );
    static const QString AppPathID( "appPath" );
    static const QString InternalPathID( "internalPath" );

    // default home paths on different platforms
#ifdef Q_OS_WIN
    static const QString PlatformDirectory( "YPlatform2" );
#else
    static const QString PlatformDirectory( ".yplatform2" );
#endif
#endif
}

//
// class:pFile
//
class pFile : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem file info" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( fileHandle_t handle READ handle WRITE setHandle )
    Q_ENUMS( PathType )
    Q_PROPERTY( PathType type READ pathType WRITE setPathType )
    Q_ENUMS( OpenModes )
    Q_PROPERTY( OpenModes mode READ mode WRITE setOpenMode )
    Q_PROPERTY( int index READ pathIndex WRITE setPathIndex )

public:
    // enums
    enum OpenModes {
        Closed = -1,
        Read,
        Write,
        Append
    };
    enum PathType {
        Directory = 0,
        Package
    };

    // property getters
    fileHandle_t    handle() const { return this->m_handle; }
    PathType        pathType() const { return this->m_type; }
    QString         name() const { return this->m_name; }
    OpenModes       mode() const { return this->m_mode; }
    int             pathIndex() const { return this->m_index; }

    // file pointers (can't be used as properties, Q_DISABLE_COPY)
    QFile           fHandle;
    pEntry          *pHandle;

public slots:
    // property setters
    void setName( const QString &name ) { this->m_name = name; }
    void setHandle( fileHandle_t handle ) { this->m_handle = handle; }
    void setPathType( PathType type ) { this->m_type = type; }
    void setOpenMode( OpenModes mode ) { this->m_mode = mode; }
    void setPathIndex( int index ) { this->m_index = index; }

private:
    // properties
    fileHandle_t    m_handle;
    PathType        m_type;
    QString         m_name;
    OpenModes       m_mode;
    int             m_index;
};

//
// class:pSearchPath
//
class pSearchPath : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem search path" )
    Q_PROPERTY( QString path READ path WRITE setPath )
    Q_PROPERTY( pPackage *package READ package WRITE setPackage )
    Q_PROPERTY( pFile::PathType type READ type WRITE setType )
    Q_PROPERTY( QString id READ id WRITE setId )

public:
    // property getters
    QString path() const { return this->m_path; }
    pPackage *package() { return this->m_package; }
    pFile::PathType type() const { return this->m_type; }
    QString id() const { return this->m_id; }

public slots:
    // property setters
    void setPath( const QString &path ) { this->m_path = path; }
    void setPackage( pPackage *package ) { this->m_package = package; }
    void setType( pFile::PathType type ) { this->m_type = type; }
    void setId( const QString &id ) { this->m_id = id; }

private:
    // properties
    QString m_path;
    pPackage *m_package;
    pFile::PathType m_type;
    QString m_id;
};

//
// class:pFileBuffer
//
class pFileBuffer : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Buffers for filesystem" )
    Q_DISABLE_COPY( pFileBuffer )
    Q_PROPERTY( QString filename READ filename WRITE setFilename )
    Q_PROPERTY( byte *buffer READ data WRITE setBuffer RESET deleteBuffer )
    Q_PROPERTY( fileHandle_t handle READ fileHandle WRITE setFileHandle )

public:
    pFileBuffer ( QString filename, byte *buffer, fileHandle_t handle ) {
        this->setFilename( filename );
        this->setBuffer( buffer );
        this->setFileHandle( handle );
    }
    QString filename() const { return this->m_filename; }
    byte *data() const { return this->m_buffer; }
    fileHandle_t fileHandle() const { return this->m_handle; }

public slots:
    void setFilename( const QString &filename ) { this->m_filename = filename; }
    void setBuffer( byte *buffer ) { this->m_buffer = buffer; }
    void deleteBuffer() { delete[] this->m_buffer; }
    void setFileHandle( const fileHandle_t handle ) { this->m_handle = handle; }

private:
    QString m_filename;
    byte *m_buffer;
    fileHandle_t m_handle;
};

//
// class:Sys_Filesystem
//
class Sys_Filesystem : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem" )
    Q_FLAGS( OpenFlags OpenFlag )
    Q_ENUMS( ListModes )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )

public:
    enum OpenFlag {
        NoFlags    = 0,
        Absolute   = 1,
        DirsOnly   = 2,
        PacksOnly  = 4,
        Force      = 8,
        Silent     = 16,
        Linked     = 32
    };
    Q_DECLARE_FLAGS ( OpenFlags, OpenFlag )
    enum SeekModes {
        Set = 0,
        Current,
        End
    };
    enum ListModes {
        ListAll = 0,
        ListDirs,
        ListFiles
    };
    bool hasInitialized() const { return this->m_initialized; }
    QList <pSearchPath*> searchPaths;
    long length( fileHandle_t ) const;
    long open( pFile::OpenModes mode, const QString &filename, fileHandle_t &fHandle, OpenFlags flags = NoFlags );
    long read( byte *buffer, unsigned long len, const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    long write( const byte *buffer, unsigned long len, const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    void close( const fileHandle_t fHandle, OpenFlags flags = NoFlags );
    void close( const QString &filename, OpenFlags flags = NoFlags );
    void print( const fileHandle_t fHandle, const QString &msg, OpenFlags flags = NoFlags );
    bool seek( const fileHandle_t fHandle, long offset, OpenFlags flags = NoFlags, SeekModes seekMode = Set );
    long readFile( const QString &filename, byte **buffer, OpenFlags flags = NoFlags );
    void touch( const QString &filename, OpenFlags flags = NoFlags );
    void defaultExtension( QString &filename, const QString &extension );
    bool extract( const QString &filename );
    bool exists( QString &path, OpenFlags &flags, int &searchPathIndex );
    bool exists( const QString &path, OpenFlags &flags, int &searchPathIndex );
    bool exists( const QString &path, OpenFlags flags, int &searchPathIndex );
    bool exists( const QString &path, OpenFlags flags = NoFlags );
    void freeFile( const QString &filename );
    QStringList list( const QString &directory, const QRegExp &filter = QRegExp(), ListModes mode = ListAll );
    pFile *fileForHandle( fileHandle_t handle ) const;

private:
    bool m_initialized;
    void addSearchPath( const QString &path, const QString id = QString::null );
    void addSearchPath( pPackage *package, const QString &filename, const QString id = QString::null );
    void openInWriteMode( const QString &filename, fileHandle_t &fHandle, OpenFlags flags = NoFlags );
    long openInAppendMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags = NoFlags );
    long openInReadMode( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, OpenFlags flags = NoFlags );
    int getSearchPathIndex( const QString &id ) const;
    void loadPackages();
    int numFileHandles;
    QList <pFile*> fileList;
    QString buildPath( const QString &filename, const QString &basePath, bool *ok );
    bool existsExt( QString &path, OpenFlags &flags, int &searchPathIndex );
    QList <pFileBuffer*> fileBuffers;
    QStringList listDirectory( const QString &searchDir, const QString &path, ListModes mode );
    // win32 link handling
#ifdef Q_OS_WIN
    bool readLink( const QString &filename, lnkInfo_t &info, OpenFlags flags );
    bool checkLink( QString &filename, int &flags );
#endif

public slots:
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void init();
    void shutdown();
    void touch();
    void list();
};

//
// externals
//
#ifndef MODULE_LIBRARY
extern class Sys_Filesystem fs;
#endif

//
// inlines for fileExists
//
inline bool Sys_Filesystem::exists( QString &path, OpenFlags &flags, int &searchPathIndex ) { return this->existsExt( path, flags, searchPathIndex ); }
inline bool Sys_Filesystem::exists( const QString &path, OpenFlags &flags, int &searchPathIndex ) { QString filename = path; return this->existsExt( filename, flags, searchPathIndex ); }
inline bool Sys_Filesystem::exists( const QString &path, OpenFlags flags, int &searchPathIndex ) { QString filename = path; return this->existsExt( filename, flags, searchPathIndex ); }
inline bool Sys_Filesystem::exists( const QString &path, OpenFlags flags ) { QString filename = path; int index; return this->existsExt( filename, flags, index ); }

#endif // SYS_FILESYSTEM_H
