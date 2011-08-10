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

//
// classes
//
class Mod_Trap mt;

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
    this->call( ModuleAPI::Platform, ModuleAPI::ComPrint, ( intptr_t )msg.toLatin1().constData());
}

/*
================
comError
================
*/
void Mod_Trap::comError( int type, const QString &msg ) {
    this->call( ModuleAPI::Platform, ModuleAPI::ComError, type, ( intptr_t )msg.toLatin1().constData());
}

/*
================
comMilliseconds
================
*/
int Mod_Trap::comMilliseconds() {
    return this->call( ModuleAPI::Platform, ModuleAPI::ComMilliseconds );
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
    return this->call( ModuleAPI::Platform, ModuleAPI::FsOpen, ( intptr_t )mode, ( intptr_t )path.toLatin1().constData(), ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsClose
=============
*/
void Mod_Trap::fsClose( const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsClose, ( intptr_t )fHandle, ( intptr_t )flags  );
}

/*
=============
fsClose
=============
*/
void Mod_Trap::fsClose( const QString &filename, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsCloseByName, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsExists
=============
*/
bool Mod_Trap::fsExists( const QString &path, Sys_Filesystem::OpenFlags flags ) {
    return this->call( ModuleAPI::Platform, ModuleAPI::FsExists, ( intptr_t )path.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsRead
=============
*/
int Mod_Trap::fsRead( byte **buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags ) {
    return this->call( ModuleAPI::Platform, ModuleAPI::FsRead, ( intptr_t )buffer, ( intptr_t )len, ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsWrite
=============
*/
int Mod_Trap::fsWrite( const byte *buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags ) {
    return this->call( ModuleAPI::Platform, ModuleAPI::FsWrite, ( intptr_t )buffer, ( intptr_t )len, ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsSeek
=============
*/
bool Mod_Trap::fsSeek( const fileHandle_t fHandle, int offset, Sys_Filesystem::OpenFlags flags ) {
    return this->call( ModuleAPI::Platform, ModuleAPI::FsSeek, ( intptr_t )fHandle, ( intptr_t )offset, ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsTouch
=============
*/
void Mod_Trap::fsTouch( const QString &filename, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsTouch, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsReadFile
=============
*/
int Mod_Trap::fsReadFile( const QString &filename, byte **buffer, Sys_Filesystem::OpenFlags flags ) {
    return this->call( ModuleAPI::Platform, ModuleAPI::FsReadFile, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )buffer, ( intptr_t )flags );
}

/*
=============
fsPrint
=============
*/
void Mod_Trap::fsPrint( const fileHandle_t fHandle, const QString &msg, Sys_Filesystem::OpenFlags flags ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsPrint, ( intptr_t )fHandle, ( intptr_t )msg.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsFreeFile
=============
*/
void Mod_Trap::fsFreeFile( const QString &filename ) {
    this->call( ModuleAPI::Platform, ModuleAPI::FsFreeFile, ( intptr_t )filename.toLatin1().constData());
}

/*
=============
fsExtract
=============
*/
bool Mod_Trap::fsExtract( const QString &filename ) {
    return this->call( ModuleAPI::Platform, ModuleAPI::FsExtract, ( intptr_t )filename.toLatin1().constData());
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
    this->call( ModuleAPI::Platform, ModuleAPI::FsList, ( intptr_t )directory.toLatin1().constData(), ( intptr_t )buffer, ( intptr_t )STRING_BUFFER_SIZE*256, ( intptr_t )filter, ( intptr_t )mode );

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
    this->call( ModuleAPI::Platform, ModuleAPI::CmdAdd, ( intptr_t )cmdName.toLatin1().constData(), ( intptr_t )cmd, ( intptr_t )description.toLatin1().constData());
}

/*
=============
cmdRemove
=============
*/
void Mod_Trap::cmdRemove( const QString &cmdName ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CmdRemove, ( intptr_t )cmdName.toLatin1().constData());
}

/*
=============
cmdArgc
=============
*/
int Mod_Trap::cmdArgc() {
    return this->call( ModuleAPI::Platform, ModuleAPI::CmdArgc );
}

/*
=============
cmdArgv
=============
*/
QString Mod_Trap::cmdArgv( int arg ) {
    char buffer[STRING_BUFFER_SIZE];

    // call platform
    this->call( ModuleAPI::Platform, ModuleAPI::CmdArgv, ( intptr_t )arg, ( intptr_t )buffer, ( intptr_t )STRING_BUFFER_SIZE );

    // return string buffer
    return QString( buffer );
}

/*
=============
cmdExecute
=============
*/
void Mod_Trap::cmdExecute( QString cmd ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CmdExecute, ( intptr_t )cmd.toLatin1().constData());
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
    bool ok = this->call( ModuleAPI::Platform, ModuleAPI::CvarCreate, ( intptr_t )name.toLatin1().constData(), ( intptr_t )string.toLatin1().constData(), ( intptr_t )flags, ( intptr_t )desc.toLatin1().constData(), ( intptr_t )&ok );

    if ( ok ) {
        this->cvars << new mCvar( name, string, flags, desc );
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
    return this->call( ModuleAPI::Platform, ModuleAPI::CvarSet, ( intptr_t )name.toLatin1().constData(), ( intptr_t )string.toLatin1().constData(), ( intptr_t )force );
}

/*
=============
cvarGet
=============
*/
QString Mod_Trap::cvarGet( const QString &name ) {
    char buffer[STRING_BUFFER_SIZE];

    // call platform
    bool result = this->call( ModuleAPI::Platform, ModuleAPI::CvarGet, ( intptr_t )name.toLatin1().constData(), ( intptr_t )buffer, ( intptr_t )STRING_BUFFER_SIZE );

    // success
    if ( result )
        return QString( buffer );
    else
        return QString( "" );
}

/*
=============
cvarReset
=============
*/
void Mod_Trap::cvarReset( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::CvarReset, ( intptr_t )name.toLatin1().constData());
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
    this->call( ModuleAPI::Platform, ModuleAPI::GuiAddAction, ( intptr_t )name.toLatin1().constData(), ( intptr_t )icon.toLatin1().constData(), ( intptr_t )callback );
}

/*
=============
guiRemoveAction
=============
*/
void Mod_Trap::guiRemoveAction( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveAction, ( intptr_t )name.toLatin1().constData());
}

/*
=============
guiAddTab
=============
*/
void Mod_Trap::guiAddTab( QWidget *widget, const QString &name, const QString &icon ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiAddTab, ( intptr_t )widget, ( intptr_t )name.toLatin1().constData(), ( intptr_t )icon.toLatin1().constData());
}

/*
=============
guiRemoveTab
=============
*/
void Mod_Trap::guiRemoveTab( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveTab, ( intptr_t )name.toLatin1().constData());
}

/*
=============
guiSetActiveTab
=============
*/
void Mod_Trap::guiSetActiveTab( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiSetActiveTab, ( intptr_t )name.toLatin1().constData());
}

/*
=============
guiSetConsoleState
=============
*/
void Mod_Trap::guiSetConsoleState( int state ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiSetConsoleState, ( intptr_t )state );
}

/*
=============
guiAddSettingsTab
=============
*/
void Mod_Trap::guiAddSettingsTab( QWidget *widget, const QString &name, const QString &icon ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiAddSettingsTab, ( intptr_t )widget, ( intptr_t )name.toLatin1().constData(), ( intptr_t )icon.toLatin1().constData());
}

/*
=============
guiRemoveSettingsTab
=============
*/
void Mod_Trap::guiRemoveSettingsTab( const QString &name ) {
    this->call( ModuleAPI::Platform, ModuleAPI::GuiRemoveSettingsTab, ( intptr_t )name.toLatin1().constData());
}

#ifndef R_BUILD
/*
=============
rLoadImage
=============
*/
imgHandle_t Mod_Trap::rLoadImage( const QString &filename ) {
    return ( imgHandle_t )this->call( ModuleAPI::Renderer, RendererAPI::LoadImage, ( intptr_t )filename.toLatin1().constData());
}

/*
=============
rDrawImage
=============
*/
void Mod_Trap::rDrawImage( float x, float y, float w, float h, float s1, float t1, float s2, float t2, imgHandle_t handle ) {
    this->call( ModuleAPI::Renderer, RendererAPI::DrawImage, (intptr_t)x, (intptr_t)y, (intptr_t)w, (intptr_t)h, (intptr_t)s1, (intptr_t)t1, (intptr_t)s2, (intptr_t)t2, (intptr_t)handle );
}

/*
=============
rDrawImage
=============
*/
void Mod_Trap::rDrawImage( float x, float y, float w, float h, imgHandle_t handle ) {
    this->rDrawImage( x, y, w, h, 0, 0, 1, 1, handle );
}

/*
=============
rLoadMaterial
=============
*/
mtrHandle_t Mod_Trap::rLoadMaterial( const QString &filename ) {
    return ( mtrHandle_t )this->call( ModuleAPI::Renderer, RendererAPI::LoadMaterial, ( intptr_t )filename.toLatin1().constData());
}

/*
=============
rDrawMaterial
=============
*/
void Mod_Trap::rDrawMaterial( float x, float y, float w, float h, mtrHandle_t handle ) {
    this->call( ModuleAPI::Renderer, RendererAPI::DrawMaterial, (intptr_t)x, (intptr_t)y, (intptr_t)w, (intptr_t)h, (intptr_t)handle );
}

/*
=============
rSetColour
=============
*/
void Mod_Trap::rSetColour( float r, float g, float b, float a ) {
    this->call( ModuleAPI::Renderer, RendererAPI::SetColour, ( intptr_t )r, ( intptr_t )g, ( intptr_t )b, ( intptr_t )a );
}

/*
=============
rLoadFont
=============
*/
fontInfo_t *Mod_Trap::rLoadFont( const QString &filename, int pointSize ) {
    return ( fontInfo_t * )this->call( ModuleAPI::Renderer, RendererAPI::LoadFont, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )pointSize );
}

#endif

