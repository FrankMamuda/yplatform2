/*
===========================================================================
Copyright (C) 2011-2012 Edd 'Double Dee' Psycho

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
// Unified scripting engine for modules
//

//
// includes
//
#include "mod_trap.h"
#include "mod_script.h"
#include "../common/sys_common.h"

/*
===================
scriptInclude
===================
*/
QScriptValue scriptInclude( QScriptContext* context, QScriptEngine* engine ) {
    Mod_ScriptEngine *mse = qobject_cast<Mod_ScriptEngine*>( engine->parent());
    QString filename;

    // check args
    if ( !context->argument( 0 ).isString()) {
        context->throwError( mse->tr( "%1 (scriptInclude): invalid args\n" ).arg( mse->name()));
        return engine->undefinedValue();
    }
    filename = context->argument( 0 ).toString();

    // set activation object
    context->setActivationObject( engine->globalObject());

    // load & evaluate script
    mse->loadScript( filename );

    // all done
    return engine->undefinedValue();
}

/*
===================
construct
===================
*/
Mod_ScriptEngine::Mod_ScriptEngine( const QString &name, const QString &path, bool contextSeparation, bool updateStack, const QString &extension ) {
    // set defaults
    this->setName( name );
    this->setPath( path );
    this->setContextSeparation( contextSeparation );
    this->setUpdateStack( updateStack );
    this->catchError( false );
    this->setInitialized( false );
    this->setDefaultExtension( extension );
    this->engine.setParent( this );

    // set global vars
    this->engine.globalObject().setProperty( "qt", this->engine.newQMetaObject( &staticQtMetaObject ));
    this->engine.globalObject().setProperty( "com", this->engine.newQObject( &com, QScriptEngine::QtOwnership ));
    this->engine.globalObject().setProperty( "Common", this->engine.newQMetaObject( &Sys_Common::staticMetaObject ));
    this->engine.globalObject().setProperty( "cmd", this->engine.newQObject( &cmd, QScriptEngine::QtOwnership ));
    this->engine.globalObject().setProperty( "cv", this->engine.newQObject( new Mse_Cvar( &this->engine ), QScriptEngine::QtOwnership ));
    this->engine.globalObject().setProperty( "Cvar", this->engine.newQMetaObject( &pCvar::staticMetaObject ));
#ifdef RENDERER_ENABLED
    this->engine.globalObject().setProperty( "r", this->engine.newQObject( &r ));
#endif
    this->engine.globalObject().setProperty( "fs", this->engine.newQObject( new Mse_Filesystem( &this->engine ), QScriptEngine::QtOwnership ));
    this->engine.globalObject().setProperty( "FileSystem", this->engine.newQMetaObject( &Sys_Filesystem::staticMetaObject ));
    this->engine.globalObject().setProperty( "File", this->engine.newQMetaObject( &pFile::staticMetaObject ));
    this->engine.globalObject().setProperty( "gui", this->engine.newQObject( &gui, QScriptEngine::QtOwnership ));
    this->engine.globalObject().setProperty( "app", this->engine.newQObject( &app, QScriptEngine::QtOwnership ));
    this->engine.globalObject().setProperty( "include", this->engine.newFunction( scriptInclude ));

    // register metatypes
    qRegisterMetaType<Sys_Common::ErrorTypes>( "Sys_Common::ErrorTypes" );
    qRegisterMetaType<pFile::OpenModes>( "pFile::OpenModes" );
    qRegisterMetaType<pFile::PathType>( "pFile::PathType" );
    qRegisterMetaType<pCvar::Flags>( "pCvar::Flags" );
    qRegisterMetaType<pCvar::AccessFlags>( "pCvar::AccessFlags" );
    qRegisterMetaType<Sys_Filesystem::OpenFlag>( "Sys_Filesystem::OpenFlag" );
    qRegisterMetaType<Sys_Filesystem::SeekModes>( "Sys_Filesystem::SeekModes" );
    qRegisterMetaType<Sys_Filesystem::ListModes>( "Sys_Filesystem::ListModes" );
    qRegisterMetaType<fileHandle_t>( "fileHandle_t" );
    qRegisterMetaType<byte*>( "byte*" );
    qRegisterMetaType<byte>( "byte" );

    // all done
    this->setInitialized();
}

/*
===================
destruct
===================
*/
Mod_ScriptEngine::~Mod_ScriptEngine() {
    if ( this->hasInitialized()) {
        this->engine.abortEvaluation();
        this->setInitialized( false );
    }
}

/*
===================
readScript
===================
*/
QString Mod_ScriptEngine::readScript( const QString &filename ) {
    QByteArray buffer;
    QString scriptText;

    // load script
    buffer = fs.readFile( filename, Sys_Filesystem::Silent );
    if ( buffer.isNull()) {
        com.error( Sys_Common::SoftError, this->tr( "%1 (loadScript): could not read script \'%2\'\n" ).arg( this->name()).arg( filename ));
        return QString::null;
    }
    scriptText = QString( buffer.data());
    buffer.clear();

    // all done
    return scriptText;
}

/*
===================
evaluateScript
===================
*/
bool Mod_ScriptEngine::evaluateScript( const QString &filename, const QString &scriptText ) {
    // perform a static check
    if ( !this->engine.canEvaluate( scriptText )) {
        com.error( Sys_Common::SoftError, this->tr( "%1 (loadScript): could not evaluate script \'%2\'\n" ).arg( this->name()).arg( filename ));
        return false;
    }
    this->engine.evaluate( scriptText );

    // uncaught exception?
    if ( this->engine.hasUncaughtException()) {
        QScriptValue exception = this->engine.uncaughtException();
        this->catchError();
        com.error( Sys_Common::SoftError,
                   this->tr( "%1 (\'%2\', line %3): \'%4\'\n" )
                   .arg( this->name())
                   .arg( filename )

                   // line number seems to be off for some reason
                   .arg( exception.property( "lineNumber" ).toInteger())
                   .arg( exception.toString())
                   );
        return false;
    }
    return true;
}

/*
===================
loadScript
===================
*/
void Mod_ScriptEngine::loadScript( const QString &filename ) {
    QString filePath = fs.defaultExtension( filename, this->defaultExtension());

    // append path if needed
    if ( !filePath.startsWith( this->path()) && !filePath.startsWith( ".." ) && !filePath.startsWith( ":/" ))
        filePath.prepend( this->path());

    // get script buffer
    QString scriptText = this->readScript( filePath );

    // create new context if required
    if ( this->hasContextSeparation())
        this->engine.pushContext();

    // perform evaluation
    if ( !this->evaluateScript( filePath, scriptText ))
        return;

    // add to global list
    this->scriptList << filename;

    // one or multiple update() functions?
    if ( !this->hasUpdateStack() || !this->hasContextSeparation())
        this->updateList.clear();

    // find update function, if there is any (not mandatory)
    this->updateList << this->engine.evaluate( "update" );

    // pop context if required
    if ( this->hasContextSeparation())
        this->engine.popContext();
}

/*
===================
update
===================
*/
void Mod_ScriptEngine::update() {
    if ( !this->hasInitialized())
        return;

    foreach ( QScriptValue updateFunc, this->updateList ) {
        if ( !updateFunc.isError() && updateFunc.isFunction() && !this->hasCaughtError()) {
            updateFunc.call( this->engine.globalObject());

            // uncaught exception?
            if ( this->engine.hasUncaughtException()) {
                QScriptValue exception = this->engine.uncaughtException();
                this->catchError();
                com.error( Sys_Common::SoftError,
                           this->tr( "%1 (update): \'%2\'\n" )
                           .arg( this->name())
                           .arg( exception.toString())
                           );
                return;
            }
        }
    }
}
