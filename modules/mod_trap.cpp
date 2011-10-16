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
// includes
//
#include "mod_trap.h"
#include "../common/sys_common.h"

//
// classes
//
class Mod_Trap mt;

/*
================
passFloat
================
*/
intptr_t Mod_Trap::passFloat( float v ) {
    floatIntUnion f;
    f.v = v;
    return static_cast<intptr_t>( f.i );
}

/*
================
getFloat
================
*/
float Mod_Trap::getFloat( intptr_t i ) {
    floatIntUnion f;
    f.i = static_cast<int>( i );
    return f.v;
}

/*
================
setPlatformCalls
================
*/
void Mod_Trap::setPlatformCalls( platformSyscallDef pSysCall ) {
    this->platformSyscall = pSysCall;
}

#ifndef R_BUILD
/*
================
setRendererCalls
================
*/
void Mod_Trap::setRendererCalls( platformSyscallDef rSysCall ) {
    this->rendererSyscall = rSysCall;
}
#endif

/*
================
callExt
================
*/
intptr_t Mod_Trap::callExt( ModuleAPI::Destination destination, int callNum, int numArgs, intptr_t *args ) {
    switch ( destination ) {
    case ModuleAPI::Platform:
        return this->platformSyscall( callNum, numArgs, args );
        break;
    case ModuleAPI::Renderer:
#ifndef R_BUILD
        return this->rendererSyscall( callNum, numArgs, args );
#endif
        break;
    }
    return false;
}


//
// syscalls:commons
//

/*
================
comPrint
================
*/
void Mod_Trap::comPrint( const QString &msg ) {
    this->call( ModuleAPI::Platform, ModuleAPI::ComPrint, reinterpret_cast<intptr_t>( msg.toLatin1().constData()));
}

/*
================
comError
================
*/
void Mod_Trap::comError( int type, const QString &msg ) {
    this->call( ModuleAPI::Platform, ModuleAPI::ComError, type, reinterpret_cast<intptr_t>( msg.toLatin1().constData()));
}

/*
================
comMilliseconds
================
*/
int Mod_Trap::comMilliseconds() {
    return static_cast<int>( this->call( ModuleAPI::Platform, ModuleAPI::ComMilliseconds ));
}

//
// syscalls:filesystem
//

/*
=============
fsOpen
=============
*/
int Mod_Trap::fsOpen( int mode, const QString &path, fileHandle_t *fHandle, Sys_Filesystem::OpenFlags flags ) {
    return static_cast<int>( this->call( ModuleAPI::Platform, ModuleAPI::FsOpen, static_cast<intptr_t>( mode ), reinterpret_cast<intptr_t>( path.toLatin1().constData()), reinterpret_cast<intptr_t>( fHandle ), static_cast<intptr_t>( flags )));
}

/*
=============
fsClose
=============
*/
void Mod_Trap::fsClose( const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsClose, static_cast<intptr_t>( fHandle ), static_cast<intptr_t>( flags ));
}

/*
=============
fsClose
=============
*/
void Mod_Trap::fsClose( const QString &filename, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsCloseByName, reinterpret_cast<intptr_t>( filename.toLatin1().constData()), static_cast<intptr_t>( flags ));
}

/*
=============
fsExists
=============
*/
bool Mod_Trap::fsExists( const QString &path, Sys_Filesystem::OpenFlags flags ) {
    return static_cast<bool>( this->call( ModuleAPI::Platform, ModuleAPI::FsExists, reinterpret_cast<intptr_t>( path.toLatin1().constData()), static_cast<intptr_t>( flags )));
}

/*
=============
fsRead
=============
*/
int Mod_Trap::fsRead( byte **buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags ) {
    return static_cast<int>( this->call( ModuleAPI::Platform, ModuleAPI::FsRead, reinterpret_cast<intptr_t>( buffer ), static_cast<intptr_t>( len ), static_cast<intptr_t>( fHandle ), static_cast<intptr_t>( flags )));
}

/*
=============
fsWrite
=============
*/
int Mod_Trap::fsWrite( const byte *buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags ) {
    return static_cast<int>( this->call( ModuleAPI::Platform, ModuleAPI::FsWrite, reinterpret_cast<intptr_t>( buffer ), static_cast<intptr_t>( len ), static_cast<intptr_t>( fHandle ), static_cast<intptr_t>( flags )));
}

/*
=============
fsSeek
=============
*/
bool Mod_Trap::fsSeek( const fileHandle_t fHandle, int offset, Sys_Filesystem::OpenFlags flags ) {
    return static_cast<bool>( this->call( ModuleAPI::Platform, ModuleAPI::FsSeek, static_cast<intptr_t>( fHandle ), static_cast<intptr_t>( offset ), static_cast<intptr_t>( fHandle ), static_cast<intptr_t>( flags )));
}

/*
=============
fsTouch
=============
*/
void Mod_Trap::fsTouch( const QString &filename, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsTouch, reinterpret_cast<intptr_t>( filename.toLatin1().constData()), static_cast<intptr_t>( flags ));
}

/*
=============
fsReadFile
=============
*/
int Mod_Trap::fsReadFile( const QString &filename, byte **buffer, Sys_Filesystem::OpenFlags flags ) {
    return static_cast<int>( this->call( ModuleAPI::Platform, ModuleAPI::FsReadFile, reinterpret_cast<intptr_t>( filename.toLatin1().constData()), reinterpret_cast<intptr_t>( buffer ), static_cast<intptr_t>( flags )));
}

/*
=============
fsPrint
=============
*/
void Mod_Trap::fsPrint( const fileHandle_t fHandle, const QString &msg, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsPrint, static_cast<intptr_t>( fHandle ), reinterpret_cast<intptr_t>( msg.toLatin1().constData()), static_cast<intptr_t>( flags ));
}

/*
=============
fsFreeFile
=============
*/
void Mod_Trap::fsFreeFile( const QString &filename ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsFreeFile, reinterpret_cast<intptr_t>( filename.toLatin1().constData()));
}

/*
=============
fsExtract
=============
*/
bool Mod_Trap::fsExtract( const QString &filename ) {
    return static_cast<bool>( this->call( ModuleAPI::Platform, ModuleAPI::FsExtract, reinterpret_cast<intptr_t>( filename.toLatin1().constData())));
}

/*
=============
fsListFiles
=============
*/
QStringList Mod_Trap::fsListFiles( const QString &directory, QRegExp *filter, Sys_Filesystem::ListModes mode ) {
    // should be enough, filelists have large buffers
    char buffer[STRING_BUFFER_SIZE*256];

    // call platform
    this->call( ModuleAPI::Platform, ModuleAPI::FsList, reinterpret_cast<intptr_t>( directory.toLatin1().constData()), reinterpret_cast<intptr_t>( buffer ), static_cast<intptr_t>( STRING_BUFFER_SIZE * 256 ), reinterpret_cast<intptr_t>( filter ), static_cast<intptr_t>( mode ));

    // create filelist
    QStringList fileList = QString( buffer ).split( ";" );

    // remove void entries
    fileList.removeAll( "" );
    return fileList;
}

//
// syscalls:cmd subsys
//
/*
=============
cmdAdd
=============
*/
void Mod_Trap::cmdAdd( const QString &cmdName, cmdCommand_t cmd, const QString &description ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CmdAdd, reinterpret_cast<intptr_t>( cmdName.toLatin1().constData()), reinterpret_cast<intptr_t>( cmd ), reinterpret_cast<intptr_t>( description.toLatin1().constData()));
}

/*
=============
cmdRemove
=============
*/
void Mod_Trap::cmdRemove( const QString &cmdName ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CmdRemove, reinterpret_cast<intptr_t>( cmdName.toLatin1().constData()));
}

/*
=============
cmdArgc
=============
*/
int Mod_Trap::cmdArgc() {
    return static_cast<int>( this->call( ModuleAPI::Platform, static_cast<intptr_t>( ModuleAPI::CmdArgc )));
}

/*
=============
cmdArgv
=============
*/
QString Mod_Trap::cmdArgv( int arg ) {
    char buffer[STRING_BUFFER_SIZE];

    // call platform
    this->call( ModuleAPI::Platform, ModuleAPI::CmdArgv, static_cast<intptr_t>( arg ), reinterpret_cast<intptr_t>( buffer ), static_cast<intptr_t>( STRING_BUFFER_SIZE ));

    // return string buffer
    return QString::fromLatin1( buffer );
}

/*
=============
cmdExecute
=============
*/
void Mod_Trap::cmdExecute( QString cmd ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CmdExecute, reinterpret_cast<intptr_t>( cmd.toLatin1().constData()));
}

//
// syscalls::cvar subsystem
//

/*
=============
cvarCreate
=============
*/
mCvar *Mod_Trap::cvarCreate( const QString &name, const QString &string, pCvar::Flags flags, const QString &desc ) {
    bool ok = static_cast<bool>( this->call( ModuleAPI::Platform, ModuleAPI::CvarCreate, reinterpret_cast<intptr_t>( name.toLatin1().constData()), reinterpret_cast<intptr_t>( string.toLatin1().constData()), static_cast<intptr_t>( flags ), reinterpret_cast<intptr_t>( desc.toLatin1().constData()), reinterpret_cast<intptr_t>( &ok )));

    if ( ok ) {
        QString actualString = mt.cvarGet( name );
        this->cvars << new mCvar( name, actualString, flags, desc );
        return this->cvars.last();
    } else
        return NULL;
}

/*
=============
cvarSet
=============
*/
bool Mod_Trap::cvarSet( const QString &name, const QString &string, bool force ) {
    return static_cast<bool>( this->call( ModuleAPI::Platform, ModuleAPI::CvarSet, reinterpret_cast<intptr_t>( name.toLatin1().constData()), reinterpret_cast<intptr_t>( string.toLatin1().constData()), static_cast<intptr_t>( force )));
}

/*
=============
cvarGet
=============
*/
QString Mod_Trap::cvarGet( const QString &name ) {
    char buffer[STRING_BUFFER_SIZE];

    // call platform
    bool result = static_cast<bool>( this->call( ModuleAPI::Platform, ModuleAPI::CvarGet, reinterpret_cast<intptr_t>( name.toLatin1().constData()), reinterpret_cast<intptr_t>( buffer ), static_cast<intptr_t>( STRING_BUFFER_SIZE )));

    // success
    if ( result )
        return QString::fromLatin1( buffer );
    else
        return QString( "" );
}

/*
=============
cvarReset
=============
*/
void Mod_Trap::cvarReset( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CvarReset, reinterpret_cast<intptr_t>( name.toLatin1().constData()));
}

//
// syscalls::applet
//

/*
=============
appShutdown
=============
*/
void Mod_Trap::appShutdown() {
    this->call( ModuleAPI::Platform, ModuleAPI::AppShutdown );
}

//
// syscalls::gui
//

/*
=============
guiRaise
=============
*/
void Mod_Trap::guiRaise() {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRaise );
}

/*
=============
guiHide
=============
*/
void Mod_Trap::guiHide() {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiHide );
}

/*
=============
guiCreateSystemTray
=============
*/
void Mod_Trap::guiCreateSystemTray() {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiCreateSystray );
}

/*
=============
guiRemoveSystemTray
=============
*/
void Mod_Trap::guiRemoveSystemTray() {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveSystray );
}

/*
=============
guiAddAction
=============
*/
void Mod_Trap::guiAddAction( const QString &name, const QString &icon, cmdCommand_t callback ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiAddAction, reinterpret_cast<intptr_t>( name.toLatin1().constData()), reinterpret_cast<intptr_t>( icon.toLatin1().constData()), reinterpret_cast<intptr_t>( callback ));
}

/*
=============
guiRemoveAction
=============
*/
void Mod_Trap::guiRemoveAction( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveAction, reinterpret_cast<intptr_t>( name.toLatin1().constData()));
}

/*
=============
guiAddTab
=============
*/
void Mod_Trap::guiAddTab( QWidget *widget, const QString &name, const QString &icon ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiAddTab, reinterpret_cast<intptr_t>( widget ), reinterpret_cast<intptr_t>( name.toLatin1().constData()), reinterpret_cast<intptr_t>( icon.toLatin1().constData()));
}

/*
=============
guiRemoveTab
=============
*/
void Mod_Trap::guiRemoveTab( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveTab, reinterpret_cast<intptr_t>( name.toLatin1().constData()));
}

/*
=============
guiSetActiveTab
=============
*/
void Mod_Trap::guiSetActiveTab( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiSetActiveTab, reinterpret_cast<intptr_t>( name.toLatin1().constData()));
}

/*
=============
guiSetConsoleState
=============
*/
void Mod_Trap::guiSetConsoleState( int state ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiSetConsoleState, static_cast<intptr_t>( state ));
}

/*
=============
guiAddSettingsTab
=============
*/
void Mod_Trap::guiAddSettingsTab( QWidget *widget, const QString &name, const QString &icon ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiAddSettingsTab, reinterpret_cast<intptr_t>( widget ), reinterpret_cast<intptr_t>( name.toLatin1().constData()), reinterpret_cast<intptr_t>( icon.toLatin1().constData()));
}

/*
=============
guiRemoveSettingsTab
=============
*/
void Mod_Trap::guiRemoveSettingsTab( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveSettingsTab, reinterpret_cast<intptr_t>( name.toLatin1().constData()));
}

#ifndef R_BUILD
#ifdef RENDERER_ENABLED
/*
=============
rLoadMaterial
=============
*/
mtrHandle_t Mod_Trap::rLoadMaterial( const QString &filename ) {
    return static_cast<mtrHandle_t>( this->call( ModuleAPI::Renderer, RendererAPI::LoadMaterial, reinterpret_cast<intptr_t>( filename.toLatin1().constData())));
}

/*
=============
rDrawMaterial
=============
*/
void Mod_Trap::rDrawMaterial( float x, float y, float w, float h, mtrHandle_t handle ) {
    this->call( ModuleAPI::Renderer, RendererAPI::DrawMaterial, this->passFloat( x ), this->passFloat( y ), this->passFloat( w ), this->passFloat( h ), static_cast<intptr_t>( handle ));
}

/*
=============
rSetColour
=============
*/
void Mod_Trap::rSetColour( float r, float g, float b, float a ) {
    this->call( ModuleAPI::Renderer, RendererAPI::SetColour, this->passFloat( r ), this->passFloat( g ), this->passFloat( b ), this->passFloat( a ));
}

/*
=============
rLoadFont
=============
*/
void Mod_Trap::rSetColour( const QColor &colour ) {
    this->rSetColour( colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF());
}

/*
=============
rDrawText

 dd: move to QRawFont on Qt 4.8?
=============
*/
void Mod_Trap::rDrawText( float x, float y, QFont *font, const QString &text, float r, float g, float b, float a ) {
    this->call( ModuleAPI::Renderer, RendererAPI::DrawText, this->passFloat( x ), this->passFloat( y ), reinterpret_cast<intptr_t>( font ), reinterpret_cast<intptr_t>( text.toLatin1().constData()),
               this->passFloat( r ),
               this->passFloat( g ),
               this->passFloat( b ),
               this->passFloat( a ));
}

/*
=============
rDrawText
=============
*/
void Mod_Trap::rDrawText( float x, float y, QFont *font, const QString &text, const QColor &colour ) {
    this->rDrawText( x, y, font, text, colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF());
}
#endif
#else
/*
=============
keyEvent
=============
*/
void Mod_Trap::keyEvent( ModuleAPI::KeyEventType type, int key ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererKeyEvent, static_cast<intptr_t>( type ), static_cast<intptr_t>( key ));
}

/*
=============
mouseEvent
=============
*/
void Mod_Trap::mouseEvent( ModuleAPI::KeyEventType type, Qt::MouseButton key ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererMouseEvent, static_cast<intptr_t>( type ), static_cast<intptr_t>( key ));
}

/*
=============
mouseMotion
=============
*/
void Mod_Trap::mouseMotion( int x, int y ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererMouseMotion, static_cast<intptr_t>( x ), static_cast<intptr_t>( y ));
}

/*
=============
mouseMotion
=============
*/
void Mod_Trap::wheelEvent( int delta ) {
    this->call( ModuleAPI::Platform, ModuleAPI::RendererWheelEvent, static_cast<intptr_t>( delta ));
}
#endif
