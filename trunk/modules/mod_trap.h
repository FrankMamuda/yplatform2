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

#ifndef MOD_TRAP_H
#define MOD_TRAP_H

//
// includes
//
#include "mod_public.h"
#include "sys_shared.h"
#include "sys_filesystem.h"
#include "sys_cmd.h"
#include "mod_cvarfunc.h"
#ifdef R_BUILD
#include "renderer_global.h"
#else
#include "module_global.h"
#ifdef RENDERER_ENABLED
#include "../renderer/r_shared.h"
#include "../renderer/r_public.h"
#include <QFont>
#include <QColor>
#endif
#endif
#include <QObject>
#include <QToolBar>
#if !defined( intptr_t )
#include <stdint.h>
#endif



//
// defines
//
typedef QVariant( *platformSyscallDef )( int, const QVariantList & );

//
// class::Mod_Trap
//
class Mod_Trap : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform communication subsystem" )

public:
    void setPlatformCalls( platformSyscallDef pSysCall );
    void setRendererCalls( platformSyscallDef pSysCall );

public slots:
    QVariant call( ModuleAPI::Destination destination, int callNum ) { return this->callExt( destination, callNum ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0 ) { QVariantList args; args << arg0; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1 ) { QVariantList args; args << arg0 << arg1; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2 ) { QVariantList args; args << arg0 << arg1 << arg2; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6, const QVariant &arg7 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6, const QVariant &arg7, const QVariant &arg8 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8; return this->callExt( destination, callNum, args ); }
    QVariant call( ModuleAPI::Destination destination, int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6, const QVariant &arg7, const QVariant &arg8, const QVariant &arg9 ) { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9; return this->callExt( destination, callNum, args ); }

    // renderer
#ifdef R_BUILD
    void keyEvent( ModuleAPI::KeyEventType, int );
    void mouseEvent( ModuleAPI::KeyEventType = ModuleAPI::KeyPress, Qt::MouseButton = Qt::LeftButton );
    void mouseMotion( int, int );
    void wheelEvent( int );
#endif

protected:
    platformSyscallDef platformSyscall;
#ifndef R_BUILD
    platformSyscallDef rendererSyscall;
#endif
    QVariant callExt( ModuleAPI::Destination destination, int callNum, const QVariantList &args = QVariantList());

signals:

public slots:

};

//
// externals
//
extern class Mod_Trap mt;

//
// class::Mod_Common
//
class Mod_Common : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Common function wrapper class" )

public slots:
    void print( const QString &msg ) { mt.call( ModuleAPI::Platform, ModuleAPI::ComPrint, msg ); }
    void error( int type, const QString &msg ) { mt.call( ModuleAPI::Platform, ModuleAPI::ComError, type, msg ); }
    int milliseconds() { return mt.call( ModuleAPI::Platform, ModuleAPI::ComMilliseconds ).toInt(); }
};

//
// class::Mod_Filesystem
//
class Mod_Filesystem : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Filesystem wrapper class" )

public:
    int open( pFile::OpenModes mode, const QString &path, fileHandle_t *fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsOpen, mode, path, qVariantFromValue( fHandle ), static_cast<int>( flags )).toInt(); }
    void close( const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { mt.call( ModuleAPI::Platform, ModuleAPI::FsClose, fHandle, static_cast<int>( flags )); }
    void close( const QString &filename, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { mt.call( ModuleAPI::Platform, ModuleAPI::FsCloseByName, filename, static_cast<int>( flags )); }
    bool exists( const QString &path, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsExists, path, static_cast<int>( flags )).toBool(); }
    int read( byte *buffer, int len, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsRead, qVariantFromValue( buffer ), len, fHandle, static_cast<int>( flags )).toInt(); }
    int write( const QByteArray buffer, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsWrite, buffer, fHandle, static_cast<int>( flags )).toInt(); }
    bool seek( const fileHandle_t fHandle, int offset, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags, Sys_Filesystem::SeekModes mode = Sys_Filesystem::Set ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsSeek, fHandle, offset, static_cast<int>( flags ), static_cast<int>( mode )).toBool(); }
    void touch( const QString &filename, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { mt.call( ModuleAPI::Platform, ModuleAPI::FsTouch, filename, static_cast<int>( flags )); }
    QByteArray readFile( const QString &filename, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsReadFile, filename, static_cast<int>( flags )).toByteArray(); }
    void print( const fileHandle_t fHandle, const QString &msg, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { mt.call( ModuleAPI::Platform, ModuleAPI::FsPrint, fHandle, msg, static_cast<int>( flags )); }
    bool extract( const QString &filename ) { return mt.call( ModuleAPI::Platform, ModuleAPI::FsExtract, filename ).toBool(); }
    QStringList listFiles( const QString &directory, const QRegExp &filter = QRegExp(), Sys_Filesystem::ListModes mode = Sys_Filesystem::ListAll );
    QString defaultExtension( const QString &filename, const QString &extension ) const { if ( !filename.endsWith( extension )) return filename + extension; else return filename; }
};

//
// class::Mod_Cmd
//
class Mod_Cmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Command subystem wrapper class" )

public:
    void add( const QString &cmdName, cmdCommand_t cmd, const QString &description = QString::null ) { mt.call( ModuleAPI::Platform, ModuleAPI::CmdAdd, cmdName, qVariantFromValue( cmd ), description ); }

public slots:
    void add( const QString &cmdName, QScriptValue cmd, const QString &description = QString::null ) { mt.call( ModuleAPI::Platform, ModuleAPI::CmdAddScripted, cmdName, qVariantFromValue( cmd ), description ); }
    void remove( const QString &cmdName ) { mt.call( ModuleAPI::Platform, ModuleAPI::CmdRemove, cmdName ); }
    void execute( const QString &cmd ) { mt.call( ModuleAPI::Platform, ModuleAPI::CmdExecute, cmd ); }
};

//
// class::Mod_Cvar
//
class Mod_Cvar : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Cvar subystem wrapper class" )

private:
    // cvars
    QList <mCvar*>cvarList;

public:
    void update( const QString &cvarName, const QString &string );
    void clear();
    mCvar *create( const QString &name, const QString &string = QString( "" ), pCvar::Flags flags = pCvar::NoFlags, const QString &description = QString::null );
    mCvar *create( const QString &name, const char *string = "", pCvar::Flags flags = pCvar::NoFlags, const QString &description = QString::null ) { return this->create( name, QString( string ), flags, description ); }
    mCvar *create( const QString &name, int value = 0, pCvar::Flags flags = pCvar::NoFlags, int min = 0, int max = 0, const QString &description = QString::null );
    mCvar *create( const QString &name, float value = 0.0f, pCvar::Flags flags = pCvar::NoFlags, float min = 0.0f, float max = 0.0f, const QString &description = QString::null );
    mCvar *create( const QString &name, bool value = false, pCvar::Flags flags = pCvar::NoFlags, const QString &description = QString::null );

public slots:
    bool set( const QString &name, const QString &string, bool force = false ) { return mt.call( ModuleAPI::Platform, ModuleAPI::CvarSet, name, string, force ).toBool(); }
    QVariant get( const QString &name ) { return mt.call( ModuleAPI::Platform, ModuleAPI::CvarGet, name ); }
    void reset( const QString &name ) { mt.call( ModuleAPI::Platform, ModuleAPI::CvarReset, name ); }
    mCvar *find( const QString &name ) const;
};

//
// class::Mod_App
//
class Mod_App : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "App common function wrapper" )

public:
    void shutdown() { mt.call( ModuleAPI::Platform, ModuleAPI::AppShutdown ); }
};

//
// class::Mod_Gui
//
class Mod_Gui : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform gui wrapper class" )

public:
    void removeAction( ModuleAPI::ToolBarActions id ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRemoveAction, static_cast<ModuleAPI::ToolBarActions>( id )); }
    void addToolBar( QToolBar *toolBarPtr ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiAddToolBar, qVariantFromValue( reinterpret_cast<void*>( toolBarPtr ))); }
    void removeToolBar( QToolBar *toolBarPtr ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRemoveToolBar, qVariantFromValue( reinterpret_cast<void*>( toolBarPtr ))); }
    void removeMainToolBar() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRemoveMainToolBar ); }
    void addTab( QWidget *widget, const QString &name, const QString &icon ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiAddTab, qVariantFromValue( widget ), name, icon ); }
    void removeTab( const QString &name ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRemoveTab, name ); }
    void setActiveTab( const QString &name ) {  mt.call( ModuleAPI::Platform, ModuleAPI::GuiSetActiveTab, name ); }
    void setConsoleState( int state ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiSetConsoleState, static_cast<int>( state )); }
    void addSettingsTab( QWidget *widget, const QString &name, const QString &icon = QString()) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiAddSettingsTab, qVariantFromValue( widget ), name, icon ); }
    void removeSettingsTab( const QString &name ) { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRemoveSettingsTab, name ); }

public slots:
    void raise() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRaise ); }
    void hide() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiHide ); }
    void createSystemTray() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiCreateSystray ); }
    void removeSystemTray() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiRemoveSystray ); }
    void showTabWidget() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiShowTabWidget ); }
    void hideTabWidget() { mt.call( ModuleAPI::Platform, ModuleAPI::GuiHideTabWidget ); }
};

//
// class::Mod_Platform
//
#ifdef RENDERER_ENABLED
class Mod_Platform : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform function wrapper class" )
    Q_DECLARE_FLAGS ( PlatformFlags, ModuleAPI::Flags )

public slots:
    PlatformFlags flags() const { return static_cast<PlatformFlags>( mt.call( ModuleAPI::Platform, ModuleAPI::PlatformFlags ).toInt()); }
};
#endif

//
// class::Mod_Renderer
//
#ifdef RENDERER_ENABLED
class Mod_Renderer : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Renderer wrapper class" )

public:
    void setColour( const QColor &colour = QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f )) { this->setColour( colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF()); }
    void drawText( float x, float y, QFont font, const QString &text, float r, float g, float b, float a = 1.0f ) { mt.call( ModuleAPI::Renderer, RendererAPI::DrawText, x, y, font, text, r, g, b, a ); }
    void drawText( float x, float y, QFont font, const QString &text, const QColor &colour = QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f )) { this->drawText( x, y, font, text, colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF()); }
    void reload() { mt.call( ModuleAPI::Renderer, RendererAPI::Reload ); }

public slots:
    mtrHandle_t loadMaterial( const QString &filename ) { return static_cast<mtrHandle_t>( mt.call( ModuleAPI::Renderer, RendererAPI::LoadMaterial, filename ).toInt()); }
    void drawMaterial( float x, float y, float w, float h, mtrHandle_t handle ) { mt.call( ModuleAPI::Renderer, RendererAPI::DrawMaterial, x, y, w, h, static_cast<int>( handle )); }
    void setColour( float r, float g, float b, float a = 1.0f ) { mt.call( ModuleAPI::Renderer, RendererAPI::SetColour, r, g, b, a ); }
    void setWindowTitle( const QString &title ) { mt.call( ModuleAPI::Renderer, RendererAPI::SetWindowTitle, title ); }
    void drawText( float x, float y, const QString &fontName, int pointSize, const QString &text, float r, float g, float b, float a = 1.0f ) { QFont font( fontName ); font.setPixelSize( pointSize ); this->drawText( x, y, font, text, r, g, b, a ); }
};
#endif

//
// externals
//
extern class Mod_Common com;
extern class Mod_Filesystem fs;
extern class Mod_Cmd cmd;
extern class Mod_Cvar cv;
extern class Mod_App app;
extern class Mod_Gui gui;
#ifdef RENDERER_ENABLED
extern class Mod_Renderer r;
extern class Mod_Platform yp2;
#endif

#endif // MOD_TRAP_H
