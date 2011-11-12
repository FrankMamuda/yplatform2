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

#ifndef SYS_MODULEFUNC_H
#define SYS_MODULEFUNC_H

//
// includes
//
#include "sys_shared.h"
#include "../renderer/r_public.h"

//
// defines
//
#if !defined( intptr_t )
#include <stdint.h>
#endif

//
// prototypes for resolvable functions
//
typedef intptr_t ( *modMainDef )( int, int, intptr_t * );
typedef void ( *modEntryDef )( intptr_t (*)( ModuleAPI::PlatformAPICalls, int, intptr_t * ));
typedef void ( *rendererEntryDef )( intptr_t (*)( RendererAPI::RendererAPICalls, int, intptr_t * ));

//
// class:pModule
//
class pModule : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform module" )
    Q_DISABLE_COPY( pModule )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( QString version READ versionString WRITE setVersionString )
    Q_PROPERTY( QString icon READ icon WRITE setIcon )
    Q_PROPERTY( bool loaded READ isLoaded WRITE setLoaded )
    Q_PROPERTY( QString error READ getErrorMessage WRITE setErrorMessage )
    Q_PROPERTY( QString filename READ filename WRITE setFilename )
    Q_PROPERTY( unsigned int api READ apiVersion WRITE setApiVersion )
    Q_ENUMS( Types )
    Q_PROPERTY( Types type READ type WRITE setType )

public:
    enum Types {
       Module = 0,
       Renderer/*,
       SoundSystem*/
    };

    // constructor
    pModule( const QString &moduleName );

    // calls
    intptr_t call( int ) const;
    intptr_t call( int, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;
    intptr_t call( int, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t, const intptr_t ) const;

    // module info
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    QString versionString() const { return this->m_version; }
    QString icon() const { return this->m_icon; }
    bool isLoaded() const { return this->m_loaded; }
    QString getErrorMessage() const { return this->m_error; }
    QString filename() const { return this->m_filename; }
    unsigned int apiVersion() const { return this->m_api; }
    Types type() const { return this->m_type; }

public slots:
    void update();
    void unload();
    void load();

    // module info
    void setName( const QString &name ) { this->m_name = name; }
    void setDescription( const QString &desc ) { this->m_description = desc; }
    void setVersionString( const QString &version ) { this->m_version = version; }
    void setIcon( const QString &icon ) { this->m_icon = icon; }
    void setLoaded( bool loaded ) { this->m_loaded = loaded; }
    void setErrorMessage( const QString &error ) { this->m_error = error; }
    void setFilename( const QString &filename ) { this->m_filename = filename; }
    void setApiVersion( unsigned int api ) { this->m_api = api; }
    void setType( Types type ) { this->m_type = type; }

private:
    void loadHandle();

    // module libHandle and funcs
    QLibrary handle;
    modMainDef modMain;
    modEntryDef entry;
    rendererEntryDef renderer;

    // module info
    QString m_name;
    QString m_description;
    QString m_version;
    QString m_icon;
    bool m_loaded;
    QString m_error;
    QString m_filename;
    unsigned int m_api;
    Types m_type;
};

//
// inlines for module calls
//
inline intptr_t pModule::call( int callNum ) const { return (( modMainDef )( this->modMain ))( callNum, 0, NULL ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0 ) const { intptr_t args[1] = { arg0 }; return (( modMainDef )( this->modMain ))( callNum, 1, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1 ) const { intptr_t args[2] = { arg0, arg1 }; return (( modMainDef )( this->modMain ))( callNum, 2, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2 ) const { intptr_t args[3] = { arg0, arg1, arg2 }; return (( modMainDef )( this->modMain ))( callNum, 3, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3 ) const { intptr_t args[4] = { arg0, arg1, arg2, arg3 }; return (( modMainDef )( this->modMain ))( callNum, 4, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4 ) const { intptr_t args[5] = { arg0, arg1, arg2, arg3, arg4 }; return (( modMainDef )( this->modMain ))( callNum, 5, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4, const intptr_t arg5 ) const { intptr_t args[6] = { arg0, arg1, arg2, arg3, arg4, arg5 }; return (( modMainDef )( this->modMain ))( callNum, 6, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4, const intptr_t arg5, const intptr_t arg6 ) const { intptr_t args[7] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6 }; return (( modMainDef )( this->modMain ))( callNum, 7, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4, const intptr_t arg5, const intptr_t arg6, const intptr_t arg7 ) const { intptr_t args[8] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 }; return (( modMainDef )( this->modMain ))( callNum, 8, args ); }
inline intptr_t pModule::call( int callNum, const intptr_t arg0, const intptr_t arg1, const intptr_t arg2, const intptr_t arg3, const intptr_t arg4, const intptr_t arg5, const intptr_t arg6, const intptr_t arg7, const intptr_t arg8 ) const { intptr_t args[9] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 }; return (( modMainDef )( this->modMain ))( callNum, 9, args ); }

#endif // SYS_MODULEFUNC_H
