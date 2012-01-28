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
typedef QVariant ( *modMainDef )( int, const QVariantList & );
typedef void ( *modEntryDef )( QVariant (*)( ModuleAPI::PlatformAPICalls, const QVariantList & ));
typedef void ( *rendererEntryDef )( QVariant (*)( RendererAPI::RendererAPICalls, const QVariantList & ));

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
    QVariant call( int callNum ) const { return (( modMainDef )( this->modMain ))( callNum, QVariantList()); }
    QVariant call( int callNum, const QVariant &arg0 ) const { QVariantList args; args << arg0; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1 ) const { QVariantList args; args << arg0 << arg1; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2 ) const { QVariantList args; args << arg0 << arg1 << arg2; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3 ) const { QVariantList args; args << arg0 << arg1 << arg2 << arg3; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4 ) const { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5 ) const { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6 ) const { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6, const QVariant &arg7 ) const { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7; return (( modMainDef )( this->modMain ))( callNum, args ); }
    QVariant call( int callNum, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5, const QVariant &arg6, const QVariant &arg7, const QVariant &arg8 ) const { QVariantList args; args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8; return (( modMainDef )( this->modMain ))( callNum, args ); }

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

#endif // SYS_MODULEFUNC_H
