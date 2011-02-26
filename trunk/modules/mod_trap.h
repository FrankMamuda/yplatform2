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

#ifndef MOD_TRAP_H
#define MOD_TRAP_H

//
// includes
//
#include "module_global.h"
#include <QObject>
#include "../../common/sys_shared.h"
#if !defined( intptr_t )
#include <stdint.h>
#endif
#include "../mod_public.h"
#include "../common/sys_filesystem.h"
#include "../common/sys_cmd.h"
#include "mod_cvarfunc.h"

//
// defines
//
typedef intptr_t( *platformSyscallDef )( int, int, intptr_t * );

//
// class::Mod_Trap
//
class Mod_Trap : public QObject {
    Q_OBJECT

public:
    void setPlatformCalls( platformSyscallDef pSysCall );

    //
    // platform calls
    //
    // commons
    void comPrint( const QString &msg );
    void comError( int type, const QString &msg );
    int comMilliseconds();
    // filesystem
    int fsfOpenFile( int mode, const QString &path, fileHandle_t *fHandle, int flags = 0 );
    void fsfCloseFile( const fileHandle_t fHandle, int flags = 0 );
    void fsfCloseFile( const QString &filename, int flags = 0 );
    bool fsFileExists( const QString &path, int flags = 0 );
    int fsRead( byte **buffer, int len, const fileHandle_t fHandle, int flags = 0 );
    int fsWrite( const byte *buffer, int len, const fileHandle_t fHandle, int flags = 0 );
    bool fsSeek( const fileHandle_t fHandle, int offset, int flags = 0 );
    void fsTouch( const QString &filename, int flags = 0 );
    int fsReadFile( const QString &filename, byte **buffer, int flags = 0 );
    void fsfPrint( const fileHandle_t fHandle, const QString &msg, int flags = 0 );
    void fsFreeFile( const QString &filename );
    bool fsExtractFromPackage( const QString &filename );
    QStringList fsListFiles( const QString &directory, QRegExp *filter = NULL, int mode = FS_LIST_ALL );
    // cmd subsystem
    void cmdAddCommand( const QString &filename, cmdCommand_t cmd, const QString &description );
    void cmdRemoveCommand( const QString &filename );
    int cmdArgc();
    QString cmdArgv( int arg );
    void cmdExecute( QString cmd );
    mCvar *cvarCreate( const QString &name, const QString &string, int flags = 0, const QString &desc = QString::null );
    bool cvarSet( const QString &name, const QString &string, bool force = false );
    void cvarReset( const QString &name );
    // app
    void appShutdown();
    // gui
    void guiRaise();
    void guiHide();
    void guiCreateSystemTray();
    void guiRemoveSystemTray();
    void guiAddAction( const QString &name, const QString &icon, cmdCommand_t callback );
    void guiRemoveAction( const QString &name );
    void guiAddTab( QWidget *widget, const QString &name, const QString &icon );
    void guiRemoveTab( const QString &name );
    void guiSetActiveTab( const QString &name );
    void guiSetConsoleState( int state );

    // cvars
    QList <mCvar*>cvars;

private:
    platformSyscallDef platformSyscall;
    intptr_t call( int callNum );
    intptr_t call( int callNum, intptr_t arg0 );
    intptr_t call( int callNum, intptr_t arg0, intptr_t arg1 );
    intptr_t call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2 );
    intptr_t call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3 );
    intptr_t call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4 );
    intptr_t call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5 );
    intptr_t call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6 );

signals:

public slots:

};

//
// inlines for Mod_Trap::call
//
inline intptr_t Mod_Trap::call( int callNum ) { return this->platformSyscall( callNum, 0, NULL ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0 ) { intptr_t args[1] = { arg0 }; return this->platformSyscall( callNum, 1, args ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0, intptr_t arg1 ) { intptr_t args[2] = { arg0, arg1 }; return this->platformSyscall( callNum, 2, args ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2 ) { intptr_t args[3] = { arg0, arg1, arg2 }; return this->platformSyscall( callNum, 3, args ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3 ) { intptr_t args[4] = { arg0, arg1, arg2, arg3 }; return this->platformSyscall( callNum, 4, args ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4 ) { intptr_t args[5] = { arg0, arg1, arg2, arg3, arg4 }; return this->platformSyscall( callNum, 5, args ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5 ) { intptr_t args[6] = { arg0, arg1, arg2, arg3, arg4, arg5 }; return this->platformSyscall( callNum, 6, args ); }
inline intptr_t Mod_Trap::call( int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6 ) { intptr_t args[7] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6 }; return this->platformSyscall( callNum, 7, args ); }


#endif // MOD_TRAP_H
