/*
===========================================================================
Copyright (C) 2011 Edd 'Double Dee' Psycho

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
loadMtrLib
===================
*/
void R_MtrLib::loadMtrLib( const QString &filename ) {
    byte *buffer;
    QString mtrLib;

    // load mtr file
    if ( !mt.fsReadFile( filename, &buffer )) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MtrLib::loadMtrLib: could not read mtrLib \'%1\'\n" ).arg( filename ));
        return;
    } else {
        if ( buffer == NULL ) {
            mt.comError( Sys_Common::SoftError, this->tr( "R_MtrLib::loadMtrLib: NULL buffer for mtrLib \'%1\'\n" ).arg( filename ));
            return;
        }
        mtrLib = QString(( const char* )buffer );
        mt.fsFreeFile( filename );
    }

    // create new context
    this->engine.pushContext();

    // do static check
    if ( !this->engine.canEvaluate( mtrLib )) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MtrLib::loadMtrLib: cannot evaluate mtrLib \'%1\'\n" ).arg( filename ));
        return;
    }
    this->engine.evaluate( mtrLib );

    // uncaught exception?
    if ( this->engine.hasUncaughtException()) {
        QScriptValue exception = this->engine.uncaughtException();
        this->catchError();
        mt.comError( Sys_Common::SoftError, this->tr( "R_MtrLib::loadMtrLib (\'%1\'): exception: \'%2\'\n" ).arg( filename ).arg( exception.toString()));
        return;
    }

    // find update function, if it is (not mandatory)
    this->updateFunc = this->engine.evaluate( "update" );

    // pop context
    this->engine.popContext();
}

/*
===================
update
===================
*/
void R_MtrLib::update() {
    if ( !this->hasInitialized())
        return;

    if ( !this->updateFunc.isError() && this->updateFunc.isFunction() && !this->hasCaughtError()) {
        this->updateFunc.call( this->object );
        if ( engine.hasUncaughtException()) {
            QScriptValue exception = engine.uncaughtException();
            mt.comError( Sys_Common::SoftError, this->tr( "R_MtrLib::loadMtrLib: could not call update(), exception \'%1\'\n" ).arg( exception.toString()));
            return;
        }
   }
}

/*
===================
scriptPrint
===================
*/
QScriptValue scriptPrint( QScriptContext *context, QScriptEngine *engine ) {
    int y;
    QString msg;

    for ( y = 0; y < context->argumentCount(); y++ ) {
        if ( y > 0 || !( y == context->argumentCount()-1 ))
            msg.append( " " );

        msg.append( context->argument( y ).toString());
    }
    mt.comPrint( msg );
    return engine->undefinedValue();
}

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
        return engine->toScriptValue((int)m.loadImage( context->argument( 0 ).toString(), mLib.getClampMode( context->argument( 1 ).toString())));
    else
        return engine->toScriptValue((int)m.loadImage( context->argument( 0 ).toString()));
}

/*
===================
getClampMode
===================
*/
R_Image::ClampModes R_MtrLib::getClampMode( const QString &mode ) {
    if ( !QString::compare( mode, "repeat", Qt::CaseInsensitive )) {
        return R_Image::Repeat;
    } else if ( !QString::compare( mode, "clamp", Qt::CaseInsensitive )) {
        return R_Image::Clamp;
    } else if ( !QString::compare( mode, "clampToEdge", Qt::CaseInsensitive ) || !QString::compare( mode, "edge", Qt::CaseInsensitive )) {
        return R_Image::ClampToEdge;
    } else {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MtrLib::setClampMode: unknown clamp mode \"%1\", setting \"repeat\"\n" ).arg( mode ));
        return R_Image::Repeat;
    }
}

/*
===================
scriptMsec
===================
*/
QScriptValue scriptMsec( QScriptContext *context, QScriptEngine *engine ) {
    Q_UNUSED( context )
    return engine->toScriptValue( mt.comMilliseconds());
}

/*
===================
scriptCvar
===================
*/
QScriptValue scriptCvar( QScriptContext *context, QScriptEngine *engine ) {
    if ( context->isCalledAsConstructor()) {
        context->throwError( QObject::trUtf8( "'cvar' called as a constructor\n" ));
        return engine->undefinedValue();
    }

    // failsafe
    if ( context->argumentCount() > 1 ) {
        context->throwError( QObject::trUtf8( "'cvar' called with multiple arguments\n" ));
        return engine->undefinedValue();
    } else if ( context->argumentCount() == 0 ) {
        context->throwError( QObject::trUtf8( "'cvar' called without arguments\n" ));
        return engine->undefinedValue();
    }

    // not yet implemented, need mt.cvarValue
    return mt.cvarGet( context->argument( 0 ).toString());
}

/*
===================
init

dd: use separate context/engine to avoid conflict?
    done, though we should handle multiple update
    funcs not just single one (QList<QScriptValue>updateFuncList smth.)
===================
*/
void R_MtrLib::init() {
    int numMtrLibFiles;

    // announce
    mt.comPrint( this->tr( "^2R_MtrLib::init: ^5loading material libraries\n" ));

    // create scripting engine
    this->object = engine.newQObject( this );

    // add custom funcs/constructors
    this->engine.globalObject().setProperty( "print", this->engine.newFunction( scriptPrint ));
    this->material = this->engine.newFunction( scriptMaterial );
    this->engine.globalObject().setProperty( "material", material );
    this->engine.globalObject().setProperty( "stage", this->engine.newFunction( scriptStage ));
    this->engine.globalObject().setProperty( "registerTexture", this->engine.newFunction( scriptTexture ));
    this->engine.globalObject().setProperty( "milliseconds", this->engine.newFunction( scriptMsec ));
    this->engine.globalObject().setProperty( "cvar", this->engine.newFunction( scriptCvar ));

    // register stage metatype
    qScriptRegisterMetaType( &this->engine, mtrStageToScriptValue, mtrStageFromScriptValue );

    // scan for material libraries, build filelist
    QRegExp filter( QString( "*.js" ));
    filter.setPatternSyntax( QRegExp::Wildcard );
    QStringList fileList = mt.fsListFiles( "materials/", &filter, Sys_Filesystem::ListFiles );

    // also add internal material libraries
    fileList << mt.fsListFiles( ":/materials/", &filter, Sys_Filesystem::ListFiles );
    numMtrLibFiles = fileList.count();

    // nothing at all?
    if ( !numMtrLibFiles ) {
        mt.comPrint( this->tr( "^3WARNING: R_Material::init: no material libraries found\n" ));
        return;
    }

    foreach ( QString filename, fileList ) {
        mt.comPrint( QString( "^1read %1\n" ).arg( filename ));
        this->loadMtrLib( filename );
    }

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
        this->engine.abortEvaluation();
        this->setInitialized( false );
    }
}
