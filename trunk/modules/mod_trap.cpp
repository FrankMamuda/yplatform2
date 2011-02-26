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
    platformSyscall = pSysCall;

}

//
// syscalls::commons
//

/*
================
comPrint
================
*/
void Mod_Trap::comPrint( const QString &msg ) {
    this->call( COM_PRINT, ( intptr_t )msg.toLatin1().constData());
}

/*
================
comError
================
*/
void Mod_Trap::comError( int type, const QString &msg ) {
    this->call( COM_ERROR, type, ( intptr_t )msg.toLatin1().constData());
}

/*
================
comMilliseconds
================
*/
int Mod_Trap::comMilliseconds() {
    return this->call( COM_MILLISECONDS );
}

//
// syscalls::filesystem
//

/*
=============
fsfOpenFile
=============
*/
int Mod_Trap::fsfOpenFile( int mode, const QString &path, fileHandle_t *fHandle, int flags ) {
    return this->call( FS_FOPEN_FILE, ( intptr_t )mode, ( intptr_t )path.toLatin1().constData(), ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsfCloseFile
=============
*/
void Mod_Trap::fsfCloseFile( const fileHandle_t fHandle, int flags ) {
    this->call( FS_FCLOSE_FILE, ( intptr_t )fHandle, ( intptr_t )flags  );
}

/*
=============
fsfCloseFile
=============
*/
void Mod_Trap::fsfCloseFile( const QString &filename, int flags ) {
    this->call( FS_FCLOSE_FILE_BY_NAME, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsFileExists
=============
*/
bool Mod_Trap::fsFileExists( const QString &path, int flags ) {
    return this->call( FS_FILE_EXISTS, ( intptr_t )path.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsRead
=============
*/
int Mod_Trap::fsRead( byte **buffer, int len, const fileHandle_t fHandle, int flags ) {
    return this->call( FS_READ, ( intptr_t )buffer, ( intptr_t )len, ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsWrite
=============
*/
int Mod_Trap::fsWrite( const byte *buffer, int len, const fileHandle_t fHandle, int flags ) {
    return this->call( FS_WRITE, ( intptr_t )buffer, ( intptr_t )len, ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsSeek
=============
*/
bool Mod_Trap::fsSeek( const fileHandle_t fHandle, int offset, int flags ) {
    return this->call( FS_SEEK, ( intptr_t )fHandle, ( intptr_t )offset, ( intptr_t )fHandle, ( intptr_t )flags );
}

/*
=============
fsTouch
=============
*/
void Mod_Trap::fsTouch( const QString &filename, int flags ) {
    this->call( FS_TOUCH, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsReadFile
=============
*/
int Mod_Trap::fsReadFile( const QString &filename, byte **buffer, int flags ) {
    return this->call( FS_READ_FILE, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )buffer, ( intptr_t )flags );
}

/*
=============
fsfPrint
=============
*/
void Mod_Trap::fsfPrint( const fileHandle_t fHandle, const QString &msg, int flags ) {
    this->call( FS_FPRINT, ( intptr_t )fHandle, ( intptr_t )msg.toLatin1().constData(), ( intptr_t )flags );
}

/*
=============
fsFreeFile
=============
*/
void Mod_Trap::fsFreeFile( const QString &filename ) {
    this->call( FS_FREE_FILE, ( intptr_t )filename.toLatin1().constData());
}

/*
=============
fsFreeFile
=============
*/
bool Mod_Trap::fsExtractFromPackage( const QString &filename ) {
    return this->call( FS_EXTRACT, ( intptr_t )filename.toLatin1().constData());
}

/*
=============
fsListFiles
=============
*/
QStringList Mod_Trap::fsListFiles( const QString &directory, QRegExp *filter, int mode ) {
    return QString(( const char* )this->call( FS_LIST_FILES, ( intptr_t )directory.toLatin1().constData(), ( intptr_t )filter, ( intptr_t )mode )).split( ";" );
}

//
// syscalls::cmd subsys
//
/*
=============
cmdAddCommand
=============
*/
void Mod_Trap::cmdAddCommand( const QString &filename, cmdCommand_t cmd, const QString &description ) {
    this->call( CMD_ADD_COMMAND, ( intptr_t )filename.toLatin1().constData(), ( intptr_t )cmd, ( intptr_t )description.toLatin1().constData());
}

/*
=============
cmdRemoveCommand
=============
*/
void Mod_Trap::cmdRemoveCommand( const QString &filename ) {
    this->call( CMD_REMOVE_COMMAND, ( intptr_t )filename.toLatin1().constData());
}

/*
=============
cmdArgc
=============
*/
int Mod_Trap::cmdArgc() {
    return this->call( CMD_ARGC );
}

/*
=============
cmdArgv
=============
*/
QString Mod_Trap::cmdArgv( int arg ) {
    return QString(( const char * )this->call( CMD_ARGV, ( intptr_t )arg ));
}

/*
=============
cmdExecute
=============
*/
void Mod_Trap::cmdExecute( QString cmd ) {
    this->call( CMD_EXECUTE, ( intptr_t )cmd.toLatin1().constData());
}

//
// syscalls::cvar subsystem
//

/*
=============
cvarCreate
=============
*/
mCvar *Mod_Trap::cvarCreate( const QString &name, const QString &string, int flags, const QString &desc ) {
    bool ok = this->call( CVAR_CREATE, ( intptr_t )name.toLatin1().constData(), ( intptr_t )string.toLatin1().constData(), ( intptr_t )flags, ( intptr_t )desc.toLatin1().constData(), ( intptr_t )&ok );

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
    return this->call( CVAR_SET, ( intptr_t )name.toLatin1().constData(), ( intptr_t )string.toLatin1().constData(), ( intptr_t )force );
}

/*
=============
cvarReset
=============
*/
void Mod_Trap::cvarReset( const QString &name ) {
    this->call( CVAR_RESET, ( intptr_t )name.toLatin1().constData());
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
    this->call( APP_SHUTDOWN );
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
    this->call( GUI_RAISE );
}

/*
=============
guiHide
=============
*/
void Mod_Trap::guiHide() {
    this->call( GUI_HIDE );
}

/*
=============
guiCreateSystemTray
=============
*/
void Mod_Trap::guiCreateSystemTray() {
    this->call( GUI_CREATE_SYSTRAY );
}

/*
=============
guiRemoveSystemTray
=============
*/
void Mod_Trap::guiRemoveSystemTray() {
    this->call( GUI_REMOVE_SYSTRAY );
}

/*
=============
guiAddAction
=============
*/
void Mod_Trap::guiAddAction( const QString &name, const QString &icon, cmdCommand_t callback ) {
    this->call( GUI_ADD_ACTION, ( intptr_t )name.toLatin1().constData(), ( intptr_t )icon.toLatin1().constData(), ( intptr_t )callback );
}

/*
=============
guiRemoveAction
=============
*/
void Mod_Trap::guiRemoveAction( const QString &name ) {
    this->call( GUI_REMOVE_ACTION, ( intptr_t )name.toLatin1().constData());
}

/*
=============
guiAddTab
=============
*/
void Mod_Trap::guiAddTab( QWidget *widget, const QString &name, const QString &icon ) {
    this->call( GUI_ADD_TAB, ( intptr_t )widget, ( intptr_t )name.toLatin1().constData(), ( intptr_t )icon.toLatin1().constData());
}

/*
=============
guiRemoveTab
=============
*/
void Mod_Trap::guiRemoveTab( const QString &name ) {
    this->call( GUI_REMOVE_TAB, ( intptr_t )name.toLatin1().constData());
}

/*
=============
guiSetActiveTab
=============
*/
void Mod_Trap::guiSetActiveTab( const QString &name ) {
    this->call( GUI_SET_ACTIVE_TAB, ( intptr_t )name.toLatin1().constData());
}

/*
=============
guiSetConsoleState
=============
*/
void Mod_Trap::guiSetConsoleState( int state ) {
    this->call( GUI_SET_CONSOLE_STATE, ( intptr_t )state );
}


