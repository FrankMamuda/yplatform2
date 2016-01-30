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
// includes
//
#include "r_main.h"
#include "r_mtrlib.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

//
// classes
//
class R_MtrLib mLib;

// declare scriptable stages for materials
Q_DECLARE_METATYPE( R_MaterialStage* )

/*
===================
scriptMaterial
===================
*/
QScriptValue scriptMaterial( QScriptContext *context, QScriptEngine *engine ) {
    // must be a constructor
    if ( !context->isCalledAsConstructor()) {
        context->throwError( QObject::trUtf8( "'material' must be as a constructor\n" ));
        return engine->undefinedValue();
    }

    // failsafe
    if ( context->argumentCount() > 1 ) {
        context->throwError( QObject::trUtf8( "'material' constructor called with multiple arguments\n" ));
        return engine->undefinedValue();
    } else if ( context->argumentCount() == 0 ) {
        context->throwError( QObject::trUtf8( "'material' constructor called without arguments\n" ));
        return engine->undefinedValue();
    }

    // create new material
    mtrHandle_t mtr = m.loadMaterial( context->argument( 0 ).toString());
    return engine->newQObject( m.mtrList.at( mtr ), QScriptEngine::ScriptOwnership );
}

/*
===================
scriptStage
===================
*/
QScriptValue scriptStage( QScriptContext *context, QScriptEngine *engine ) {
    // can be a constructor, but not mandatory
    if ( context->argumentCount() > 3 ) {
        context->throwError( QObject::trUtf8( "'stage' constructor called with >3 arguments\n" ));
        return engine->undefinedValue();
    }

    // create new stage
    if ( !context->argumentCount())
        return engine->newQObject( new R_MaterialStage(), QScriptEngine::ScriptOwnership );
    else if ( context->argumentCount() == 1 )
        // stage( texture )
        return engine->newQObject( new R_MaterialStage( context->argument( 0 ).toString()), QScriptEngine::ScriptOwnership );
    else if ( context->argumentCount() == 2 ) {
        if ( context->argument( 1 ).isString())
            // stage( texture, clampMode )
            return engine->newQObject( new R_MaterialStage( context->argument( 0 ).toString(), context->argument( 1 ).toString()), QScriptEngine::ScriptOwnership );
        else {
            // stage( texture, mtrPtr )
            R_Material *mtrPtr = qobject_cast<R_Material*>( context->argument( 1 ).toQObject());
            return engine->newQObject( new R_MaterialStage( context->argument( 0 ).toString(), mtrPtr ), QScriptEngine::ScriptOwnership );
        }
    } else if ( context->argumentCount() == 3 ) {
        if ( context->argument( 2 ).isString()) {
            // stage( texture, mtrPtr, clampMode )
            R_Material *mtrPtr = qobject_cast<R_Material*>( context->argument( 1 ).toQObject());
            return engine->newQObject( new R_MaterialStage( context->argument( 0 ).toString(), mtrPtr, context->argument( 2 ).toString()), QScriptEngine::ScriptOwnership );
        } else
            context->throwError( QObject::trUtf8( "unknown 'stage' constructor\n" ));
    }

    return engine->undefinedValue();
}

/*
===================
mtrStageToScriptValue
===================
*/
QScriptValue mtrStageToScriptValue( QScriptEngine *engine, R_MaterialStage* const &mtrStage ) {
    return engine->newQObject( mtrStage );
}

/*
===================
mtrStageFromScriptValue
===================
*/
void mtrStageFromScriptValue( const QScriptValue &object, R_MaterialStage* &mtrStage ) {
    mtrStage = qobject_cast<R_MaterialStage*>( object.toQObject());
}

/*
===================
scriptTexture
===================
*/
QScriptValue scriptTexture( QScriptContext *context, QScriptEngine *engine ) {
    if ( context->isCalledAsConstructor()) {
        context->throwError( QObject::trUtf8( "'registerTexture' called as a constructor\n" ));
        return engine->undefinedValue();
    }

    // failsafe
    if ( context->argumentCount() > 2 ) {
        context->throwError( QObject::trUtf8( "'registerTexture' called with >2 arguments\n" ));
        return engine->undefinedValue();
    } else if ( context->argumentCount() == 0 ) {
        context->throwError( QObject::trUtf8( "'registerTexture' called without arguments\n" ));
        return engine->undefinedValue();
    }

    // create new material
    if ( context->argumentCount() == 2 )
        return engine->toScriptValue( static_cast<int>( m.loadTexture( context->argument( 0 ).toString()), mLib.getClampMode( context->argument( 1 ).toString())));
    else
        return engine->toScriptValue( static_cast<int>( m.loadTexture( context->argument( 0 ).toString())));
}

/*
===================
getClampMode
===================
*/
R_Texture::WrapMode R_MtrLib::getClampMode( const QString &mode ) {
    if ( !QString::compare( mode, "repeat", Qt::CaseInsensitive )) {
        return R_Texture::Repeat;
    } else if ( !QString::compare( mode, "clamp", Qt::CaseInsensitive )) {
        return R_Texture::ClampToBorder;
    } else if ( !QString::compare( mode, "clampToEdge", Qt::CaseInsensitive ) || !QString::compare( mode, "edge", Qt::CaseInsensitive )) {
        return R_Texture::ClampToEdge;
    } else {
        com.error( StrSoftError + this->tr( "unknown clamp mode \"%1\", setting \"repeat\"\n" ).arg( mode ));
        return R_Texture::Repeat;
    }
}

/*
===================
init
===================
*/
void R_MtrLib::init() {
    int numMtrLibFiles;
    QScriptEngine *e;

    // announce
    com.print( StrMsg + this->tr( "loading material libraries\n" ));

    // init scripting engine
    this->mse = new Mod_ScriptEngine( "MtrLib", "materials/", true, true );

    // add custom funcs/constructors
    e = &mse->engine;
    e->globalObject().setProperty( "material", e->newFunction( scriptMaterial ));
    e->globalObject().setProperty( "stage", e->newFunction( scriptStage ));
    e->globalObject().setProperty( "registerTexture", e->newFunction( scriptTexture ));

    // register stage metatype
    qScriptRegisterMetaType( e, mtrStageToScriptValue, mtrStageFromScriptValue );

    // scan for material libraries, build filelist
    QRegExp filter( QString( "*.js" ));
    filter.setPatternSyntax( QRegExp::Wildcard );
    QStringList fileList = fs.listFiles( "materials/", filter, Sys_Filesystem::ListFiles );

    // also add internal material libraries
    fileList << fs.listFiles( ":/materials/", filter, Sys_Filesystem::ListFiles );
    numMtrLibFiles = fileList.count();

    // nothing at all?
    if ( !numMtrLibFiles ) {
        com.print( StrWarn + this->tr( "no material libraries found\n" ));
        return;
    }

    // parse one by one
    foreach ( QString filename, fileList )
        this->mse->loadScript( filename );

    // all done
    this->setInitialized();
}

/*
===================
shutdown
===================
*/
void R_MtrLib::shutdown() {
    if ( this->hasInitialized()) {
        this->mse->~Mod_ScriptEngine();
        this->setInitialized( false );
    }
}
