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

#ifndef MOD_SCRIPT_H
#define MOD_SCRIPT_H

//
// includes
//
#include "mod_public.h"
#include "sys_shared.h"
#include "sys_filesystem.h"
#include "sys_cmd.h"
#include "mod_trap.h"
#include "mod_cvarfunc.h"

//
// namespace:ModScriptEngine
//
namespace MSE {
    static const QString DefaultName( "ModScriptEngine" );
    static const QString DefaultPath( "scripts/" );
    static const QString DefaultExtension( ".js" );
}

//
// class:Mod_ScriptEngine
//
class Mod_ScriptEngine : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Module scripting engine" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString path READ path WRITE setPath )
    Q_PROPERTY( bool updateStack READ hasUpdateStack WRITE setUpdateStack )
    Q_PROPERTY( bool contextSeparation READ hasContextSeparation WRITE setContextSeparation )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_PROPERTY( bool scriptError READ hasCaughtError WRITE catchError )
    Q_PROPERTY( QString defaultExtension READ defaultExtension WRITE setDefaultExtension )

public:
    explicit Mod_ScriptEngine( const QString &name = MSE::DefaultName, const QString &path = MSE::DefaultPath, bool contextSeparation = false, bool updateStack = true, const QString &extension = MSE::DefaultExtension );
    ~Mod_ScriptEngine();

    // property getters
    QString name() const { return this->m_name; }
    QString path() const { return this->m_path; }
    bool hasUpdateStack() const { return this->m_updateStack; }
    bool hasContextSeparation() const { return this->m_contextSeparation; }
    bool hasInitialized() const { return this->m_initialized; }
    bool hasCaughtError() const { return this->m_scriptError; }
    QString defaultExtension() const { return this->m_defaultExtension; }

    // scripting engine
    QScriptEngine engine;

    // script engine specific
    void loadScript( const QString &filename );
    QString readScript( const QString &filename );
    bool evaluateScript( const QString &filename, const QString &scriptText );
    QList<QScriptValue> updateList;
    QStringList scriptList;

signals:

public slots:
    // property setters
    void setName( const QString &name = MSE::DefaultName ) { this->m_name = name; }
    void setPath( const QString &path = MSE::DefaultPath ) { this->m_path = path; if ( !this->m_path.endsWith( '/' )) this->m_path.append( '/' ); }
    void setUpdateStack( bool stack = true ) { this->m_updateStack = stack; }
    void setContextSeparation( bool sep = true ) { this->m_contextSeparation = sep; }
    void setInitialized( bool initialized = true ) { this->m_initialized = initialized; }
    void catchError( bool error = true ) { this->m_scriptError = error; }
    void setDefaultExtension( const QString &extension = MSE::DefaultExtension ) { this->m_defaultExtension = extension; }
    void update();

private:
    // properties
    QString m_name;
    QString m_path;
    bool m_updateStack;
    bool m_contextSeparation;
    bool m_initialized;
    bool m_scriptError;
    QString m_defaultExtension;
};

//
// class::Mse_Cvar (scripts handle cvars a little differently - we do not expose mCvar* to script engine)
//
class Mse_Cvar : public QObject {
    Q_OBJECT

public:
    Mse_Cvar( QScriptEngine *e ) { this->engine = e; }

private:
    QScriptEngine *engine;

public slots:
    void create( const QString &name, const QString &string, pCvar::Flags flags = pCvar::NoFlags, const QString &desc = QString::null ) { cv.create( name, string, flags, desc ); }
    bool set( const QString &name, const QString &string, pCvar::AccessFlags flags = false ) { return cv.set( name, string, flags ); }
    QString get( const QString &name ) { return cv.get( name ); }
    void reset( const QString &name ) { cv.reset( name ); }
};


//
// CANT WE JUST MERGE THIS WITH MOD_TRAP WITH A SPECIAL DEFINE???
//

//
// class::Mse_Filesystem (script optimized filesystem implementation)
//
class Mse_Filesystem : public QObject {
    Q_OBJECT

public:
    Mse_Filesystem( QScriptEngine *e ) { this->engine = e; }

private:
    QScriptEngine *engine;

public slots:
    QScriptValue open( int mode, const QString &path, int flags = Sys_Filesystem::NoFlags ) {
        fileHandle_t handle;
        int length;
        length = fs.open( static_cast<pFile::OpenModes>( mode ), path, &handle, static_cast<Sys_Filesystem::OpenFlags>( flags ));

        // create return value
        QScriptValue fileDescriptor = this->engine->newObject();
        fileDescriptor.setProperty( "handle", handle );
        fileDescriptor.setProperty( "length", length );
        fileDescriptor.setProperty( "mode", mode );
        fileDescriptor.setProperty( "flags", static_cast<int>( flags ));
        return fileDescriptor;

    }
    void close( const fileHandle_t fHandle, int flags = Sys_Filesystem::NoFlags ) { fs.close( fHandle, static_cast<Sys_Filesystem::OpenFlags>( flags )); }
    void close( const QString &filename, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { fs.close( filename, static_cast<Sys_Filesystem::OpenFlags>( flags )); }
    bool exists( const QString &path, int flags = Sys_Filesystem::NoFlags ) { return fs.exists( path, static_cast<Sys_Filesystem::OpenFlags>( flags )); }
    QScriptValue read( int len, const fileHandle_t fHandle, int flags = Sys_Filesystem::NoFlags ) {
        int length;
        byte *buffer = new byte[len+1];

        // add trailing zero
        buffer[len] = '\0';
        length = fs.read( buffer, len, fHandle, static_cast<Sys_Filesystem::OpenFlags>( flags ));

        // create return value
        QScriptValue readBuffer = this->engine->newObject();
        readBuffer.setProperty( "length", length );
        readBuffer.setProperty( "buffer", reinterpret_cast<const char*>( buffer ));
        return readBuffer;
    }
    void touch( const QString &filename, int flags = Sys_Filesystem::NoFlags ) { fs.touch( filename, static_cast<Sys_Filesystem::OpenFlags>( flags )); }
    QScriptValue readFile( const QString &filename, int flags = Sys_Filesystem::NoFlags ) {
        QByteArray data = fs.readFile( filename, static_cast<Sys_Filesystem::OpenFlags>( flags ));

        // create return value
        QScriptValue readBuffer = this->engine->newObject();
        readBuffer.setProperty( "length", data.length());
        readBuffer.setProperty( "buffer", data.data());
        return readBuffer;
    }
    bool seek( const fileHandle_t fHandle, int offset, int flags = Sys_Filesystem::NoFlags, int mode = Sys_Filesystem::Set ) { return fs.seek( fHandle, offset, static_cast<Sys_Filesystem::OpenFlags>( flags ), static_cast<Sys_Filesystem::SeekModes>( mode )); }
    void print( const fileHandle_t fHandle, const QString &msg, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { fs.print( fHandle, msg, static_cast<Sys_Filesystem::OpenFlags>( flags )); }
    bool extract( const QString &filename ) { return fs.extract( filename ); }
    QString defaultExtension( const QString &filename, const QString &extension ) const { return fs.defaultExtension( filename, extension ); }
    QScriptValue listFiles( const QString &directory, const QScriptValue &filter = QScriptValue(), int mode = Sys_Filesystem::ListAll ) {
        QStringList fileList = fs.listFiles( directory, filter.toRegExp(), static_cast<Sys_Filesystem::ListModes>( mode ));
        QString merged( "" );

        foreach ( QString file, fileList )
            merged.append( QString( "\"%1\",").arg( file ));
        merged.remove( merged.length()-1, 1 );

        // create return value
        QScriptValue array;
        array = this->engine->evaluate( QString( "new Array( %1 )" ).arg( merged ));
        return array;
    }
    int write( QScriptValue buffer, const fileHandle_t fHandle, Sys_Filesystem::OpenFlags flags = Sys_Filesystem::NoFlags ) { return fs.write( buffer.toVariant().toByteArray(), fHandle, static_cast<Sys_Filesystem::OpenFlags>( flags )); }
};

#endif // MOD_SCRIPT_H
