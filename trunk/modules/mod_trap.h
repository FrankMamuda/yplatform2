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
#ifdef R_BUILD
#include "renderer_global.h"
#include "../common/sys_shared.h"
#include "../modules/mod_public.h"
#include <QtScript>
#else
#include "module_global.h"
#include "../../common/sys_shared.h"
#include "../mod_public.h"
#ifdef RENDERER_ENABLED
#include "../renderer/r_shared.h"
#include "../renderer/r_public.h"
#include <QFont>
#include <QColor>
#endif
#endif
#include <QObject>
#if !defined( intptr_t )
#include <stdint.h>
#endif
#include "../common/sys_filesystem.h"
#include "../common/sys_cmd.h"
#include "mod_cvarfunc.h"

//
// defines
//
#define STRING_BUFFER_SIZE 4096
typedef intptr_t( *platformSyscallDef )( int, int, intptr_t * );

//
// class::Mod_Trap
//
class Mod_Trap : public QObject {
    Q_OBJECT

public:
    // float conversion struct
    typedef union {
            float v;
            int i;
            unsigned int ui;
    } floatIntUnion;

    void setPlatformCalls( platformSyscallDef pSysCall );
    void setRendererCalls( platformSyscallDef pSysCall );
    intptr_t passFloat( float v );
    float getFloat( intptr_t i );

    // cvars
    QList <mCvar*>cvars;

public slots:
    //
    // platform calls
    //
    // commons
    void comPrint( const QString &msg );
    void comError( int type, const QString &msg );
    int comMilliseconds();

    // filesystem
    int fsOpen( int mode, const QString &path, fileHandle_t *fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    void fsClose( const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    void fsClose( const QString &filename, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    bool fsExists( const QString &path, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    int fsRead( byte **buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    int fsWrite( const byte *buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    bool fsSeek( const fileHandle_t fHandle, int offset, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    void fsTouch( const QString &filename, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    int fsReadFile( const QString &filename, byte **buffer, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    void fsPrint( const fileHandle_t fHandle, const QString &msg, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags );
    void fsFreeFile( const QString &filename );
    bool fsExtract( const QString &filename );
    QStringList fsListFiles( const QString &directory, QRegExp *filter = NULL, Sys_Filesystem::ListModes mode = Sys_Filesystem::ListAll );
    // cmd subsystem
    void cmdAdd( const QString &cmdName, cmdCommand_t cmd, const QString &description );
    void cmdRemove( const QString &cmdName );
    int cmdArgc();
    QString cmdArgv( int arg );
    void cmdExecute( QString cmd );
    mCvar *cvarCreate( const QString &name, const QString &string, pCvar::Flags = pCvar::NoFlags, const QString &desc = QString::null );
    bool cvarSet( const QString &name, const QString &string, bool force = false );
    QString cvarGet( const QString &name );
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
    void guiAddSettingsTab( QWidget *widget, const QString &name, const QString &icon = QString());
    void guiRemoveSettingsTab( const QString &name );
    // renderer
#ifndef R_BUILD
#ifdef RENDERER_ENABLED
    mtrHandle_t rLoadMaterial( const QString &filename );
    void rDrawMaterial( float x, float y, float w, float h, mtrHandle_t handle );
    void rSetColour( float r, float g, float b, float a = 1.0f );
    void rSetColour( const QColor &colour = QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ));
    void rDrawText( float x, float y, QFont *font, const QString &text, float r, float g, float b, float a = 1.0f );
    void rDrawText( float x, float y, QFont *font, const QString &text, const QColor &colour = QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ));
#endif
#else
    void keyEvent( ModuleAPI::KeyEventType, int );
    void mouseEvent( ModuleAPI::KeyEventType = ModuleAPI::KeyPress, Qt::MouseButton = Qt::LeftButton );
    void mouseMotion( int, int );
    void wheelEvent( int );
#endif

private:
    platformSyscallDef platformSyscall;
#ifndef R_BUILD
    platformSyscallDef rendererSyscall;
#endif
    intptr_t callExt( ModuleAPI::Destination destination, int callNum, int numArgs, intptr_t *args );
    intptr_t call( ModuleAPI::Destination destination, int callNum );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg9 );
    intptr_t call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8, intptr_t arg9 );

signals:

public slots:

};

//
// inlines for Mod_Trap::call
//
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum ) { return this->callExt( destination, callNum, 0, NULL ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0 ) { intptr_t args[1] = { arg0 }; return this->callExt( destination, callNum, 1, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1 ) { intptr_t args[2] = { arg0, arg1 }; return this->callExt( destination, callNum, 2, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2 ) { intptr_t args[3] = { arg0, arg1, arg2 }; return this->callExt( destination, callNum, 3, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3 ) { intptr_t args[4] = { arg0, arg1, arg2, arg3 }; return this->callExt( destination, callNum, 4, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4 ) { intptr_t args[5] = { arg0, arg1, arg2, arg3, arg4 }; return this->callExt( destination, callNum, 5, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5 ) { intptr_t args[6] = { arg0, arg1, arg2, arg3, arg4, arg5 }; return this->callExt( destination, callNum, 6, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6 ) { intptr_t args[7] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6 }; return this->callExt( destination, callNum, 7, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7 ) { intptr_t args[8] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 }; return this->callExt( destination, callNum, 8, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8 ) { intptr_t args[9] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 }; return this->callExt( destination, callNum, 9, args ); }
inline intptr_t Mod_Trap::call( ModuleAPI::Destination destination, int callNum, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8, intptr_t arg9 ) { intptr_t args[10] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 }; return this->callExt( destination, callNum, 10, args ); }

//
// externals
//
extern class Mod_Trap mt;

#endif // MOD_TRAP_H
