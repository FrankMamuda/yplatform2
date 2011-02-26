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

// we exclusively use forward slashes in fs, since QFile also supports these
#define FS_PATH_SEPARATOR '/'
#define FS_PACKAGE_EXTENSION ".package"
#define FS_PACKAGE_MAXPATH 256

#define FS_FLAGS_NONE       0
#define FS_FLAGS_ABSOLUTE   1
#define FS_FLAGS_DIRSONLY   2
#define FS_FLAGS_PACKSONLY  4
#define FS_FLAGS_FORCE      8
#define FS_FLAGS_SILENT     16
#define FS_FLAGS_LINKED     32

typedef int fileHandle_t;

enum {
    FS_MODE_CLOSED = -1,
    FS_MODE_READ,
    FS_MODE_WRITE,
    FS_MODE_APPEND
};

enum {
    FS_SEEK_SET = 0,
    FS_SEEK_CURRENT,
    FS_SEEK_END
};

enum {
    FS_LIST_ALL = 0,
    FS_LIST_DIRS_ONLY,
    FS_LIST_FILES_ONLY
};


#ifndef MODULE_LIBRARY
#define FS_BASE_DIRECTORY "base"
#ifdef Q_OS_WIN
#define FS_PLATFORM_DIRECTORY "YPlatform2"
#else
#define FS_PLATFORM_DIRECTORY ".yplatform2"
#endif

#define FS_HOMEPATH_ID  "homePath"
#define FS_APPPATH_ID   "appPath"

#define FS_PACKAGE_SEEK_BUFFER 65536

enum {
    SEARCHPATH_DIRECTORY,
    SEARCHPATH_PACKAGE
};

typedef struct packedFileInfo_s {
    QString name;
    uLong pos;
} packedFileInfo_t;

typedef struct package_s {
    int searchPathIndex;
    pkgFile handle;
    QList<packedFileInfo_t*>fileList;
} package_t;

typedef struct seachPath_s {
    QString path;
    package_t *package;
    int type;
    QString id;
} searchPath_t;

typedef struct fsFileInfo_s {
    fileHandle_t handle;
    QFile       fHandle;
    pkgFile     pHandle;
    int         type;
    QString     filename;
    int         openMode;
    uLong       packagePos;
    int         searchPathIndex;
} fsFileInfo_t;

#ifdef Q_OS_WIN32
// or rename to smth else?
#define FS_LNK_EXT ".lnk"

typedef struct lnkInfo_s {
    lnkInfo_s() : fileSize(0), isNetworkPath( false ), isDirectory( false ), isFileOrDir( false ) {
    }
    quint32 fileSize;
    bool isNetworkPath;
    bool isDirectory;
    bool isFileOrDir;
    QString volumeName;
    QString driveName;
    QString path;
    QString description;
} lnkInfo_t;

//TODO: little/big endian problem ?
typedef struct lnkHeader_s {
    char magic[4];
    char GUID[16];
    quint32 flags;
    quint32 attributes;
    char time1[8];
    char time2[8];
    char time3[8];
    quint32 length;
    quint32 iconNum;
    quint32 showWnd;
    quint32 hotKey;
    char filler[8];
} lnkHeader_t;

typedef struct lnkFileLocation_s {
    quint32 totalLen;
    quint32 ptr;
    quint32 flags;
    quint32 localVolume;
    quint32 basePath;
    quint32 netVolume;
    quint32 pathname;
} lnkFileLocation_t;
#endif

//
// class::YFileBuffer
//
class YFileBuffer {
public:
    YFileBuffer ( QString name, byte *buf, fileHandle_t fHandle ) {
        this->filename = name;
        this->buffer = buf;
        this->handle = fHandle;
    }
    QString filename;
    fileHandle_t handle;
    byte *buffer;
};

//
// class::Sys_Filesystem
//
class Sys_Filesystem : public QObject {
    Q_OBJECT

private:
    void addSearchPath( const QString &path, const QString id = QString::null );
    void addSearchPath( package_t *package, const QString &filename, const QString id = QString::null );
    void fOpenFileWrite( const QString &filename, fileHandle_t &fHandle, int flags = FS_FLAGS_NONE );
    int  fOpenFileAppend( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, int flags = FS_FLAGS_NONE );
    int  fOpenFileRead( const QString &filename, fileHandle_t &fHandle, int searchPathIndex, int flags = FS_FLAGS_NONE );
    int getSearchPathIndex( const QString &id );
    void loadPackages();
    package_t *loadPackage( const QString &packageName, int searchPathIndex );
    int numFileHandles;
    QList <fsFileInfo_t*> fileList;
    QString buildPath( const QString &filename, const QString &basePath, bool *ok );
    bool fileExistsExt( QString &path, int &flags, int &searchPathIndex );
    QList <YFileBuffer*> fileBuffers;
#ifdef Q_OS_WIN32
    bool readLink( const QString &filename, lnkInfo_t &info, int flags );
    bool checkLink( QString &filename, int &flags );
#endif

public:
    bool initialized;
    QList <searchPath_t*> searchPaths;
    int fOpenFile( int mode, const QString &filename, fileHandle_t &fHandle, int flags = FS_FLAGS_NONE );
    int read( byte *buffer, int len, const fileHandle_t fHandle, int flags = FS_FLAGS_NONE );
    int write( const byte *buffer, int len, const fileHandle_t fHandle, int flags = FS_FLAGS_NONE );
    void fCloseFile( const fileHandle_t fHandle, int flags = FS_FLAGS_NONE );
    void fCloseFile( const QString &filename, int flags = FS_FLAGS_NONE );
    void fPrint( const fileHandle_t fHandle, const QString &msg, int flags = FS_FLAGS_NONE );
    bool seek( const fileHandle_t fHandle, long offset, int flags = FS_FLAGS_NONE, int seekMode = FS_SEEK_SET );
    int readFile( const QString &filename, byte **buffer, int flags = FS_FLAGS_NONE );
    void touch( const QString &filename, int flags = FS_FLAGS_NONE );
    void defaultExtension( QString &filename, const QString &extension );
    bool extractFromPackage( const QString &filename );
    bool fileExists( QString &path, int &flags, int &searchPathIndex );
    bool fileExists( const QString &path, int &flags, int &searchPathIndex );
    bool fileExists( const QString &path, int flags, int &searchPathIndex );
    bool fileExists( const QString &path, int flags = 0 );
    //void freeFile( const fileHandle_t fHandle );
    void freeFile( const QString &filename );
    QStringList list( const QString &directory, const QRegExp &filter = QRegExp(), int mode = FS_LIST_ALL );

public slots:
    void init();
    void shutdown();
    void touch();
    void list();
};

//
// inlines for fileExists
//
inline bool Sys_Filesystem::fileExists( QString &path, int &flags, int &searchPathIndex ) { return this->fileExistsExt( path, flags, searchPathIndex ); }
inline bool Sys_Filesystem::fileExists( const QString &path, int &flags, int &searchPathIndex ) { QString filename = path; return this->fileExistsExt( filename, flags, searchPathIndex ); }
inline bool Sys_Filesystem::fileExists( const QString &path, int flags, int &searchPathIndex ) { QString filename = path; return this->fileExistsExt( filename, flags, searchPathIndex ); }
inline bool Sys_Filesystem::fileExists( const QString &path, int flags ) { QString filename = path; int index; return this->fileExistsExt( filename, flags, index ); }

#endif
#endif // SYS_FILESYSTEM_H
