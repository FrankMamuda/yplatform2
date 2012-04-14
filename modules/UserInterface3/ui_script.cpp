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
#include "../mod_trap.h"
#include "ui_script.h"
#include "ui_item.h"
#include "../../common/sys_common.h"
#include "../mod_cvarfunc.h"

//
// cmds
//
createCommandPtr( m.uiS, loadUIScript )
createCommandPtr( m.uiS, evaluateUIScript )

/*
===================
evaluateUIScript
===================
*/
void UI_ScriptEngine::evaluateUIScript( const QStringList &args ) {
    // check args
    if ( args.count() < 1 ) {
        com.print( this->tr( "^3usage: ^2ui3_evaluateScript ^3[^2filename^3]\n" ));
        return;
    }
    this->mse->evaluateScript( "(internal)", args.first());
}

/*
===================
loadUIScript
===================
*/
void UI_ScriptEngine::loadUIScript( const QStringList &args ) {
    // check args
    if ( args.count() < 1 ) {
        com.print( this->tr( "^3usage: ^2ui3_loadScript ^3[^2filename^3]\n" ));
        return;
    }
    this->mse->loadScript( args.first());
}

/*
===================
scriptText
===================
*/
QScriptValue scriptText( QScriptContext *context, QScriptEngine *engine ) {
    UiItem *itemPtr = NULL;
    QScriptContextInfo info( context );

    // must be a constructor
    if ( !context->isCalledAsConstructor()) {
        context->throwError( QObject::trUtf8( "line %1: 'text' must be a constructor\n" ).arg( info.lineNumber()));
        return engine->undefinedValue();
    }

    // construct item with text
    if ( context->argumentCount() >= 1 ) {
        if ( !context->argument( 0 ).isString())
            goto fail;

        // text( text )
        itemPtr = new UiItem( UiItem::Text, context->argument( 0 ).toString());
        itemPtr->setText( context->argument( 0 ).toString());
        itemPtr->setPointSize( 12 );
        itemPtr->setLeftAligned();
        itemPtr->setFontFamily( "Sans" );

        if ( context->argumentCount() >= 2 ) {
            // text( text, fontName )
            if ( context->argument( 1 ).isString()) {
                itemPtr->setFontFamily( context->argument( 1 ).toString());
            }
            // text( text, parent )
            else if ( context->argument( 1 ).isQObject() && context->argumentCount() == 2 ) {
                itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 1 ).toQObject()));
            }
            // text( text, pointSize )
            else if ( context->argument( 1 ).isNumber()) {
                itemPtr->setPointSize( context->argument( 1 ).toInteger());
            } else
                goto fail;

            if ( context->argumentCount() >= 3 ) {
                if ( !context->argument( 1 ).isQObject()) {
                    // text( text, anythingButParent, parent )
                    if ( context->argument( 2 ).isQObject() && context->argumentCount() == 3 ) {
                        itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 2 ).toQObject()));
                    } else
                        goto fail;
                } else
                    goto fail;
            }
        }

        return engine->newQObject( itemPtr, QScriptEngine::QtOwnership );
    }
    // container()
    else if ( context->argumentCount() == 0 )
        itemPtr = new UiItem( UiItem::Text );

fail:
    com.error( Sys_Common::SoftError,
               QObject::trUtf8( "line %1: invalid 'text' constructor\n" )
               .arg( info.lineNumber())
               );
    return engine->undefinedValue();
}

/*
===================
scriptWindow
===================
*/
QScriptValue scriptWindow( QScriptContext *context, QScriptEngine *engine ) {
    UiItem *itemPtr = NULL;
    QScriptContextInfo info( context );

    // must be a constructor
    if ( !context->isCalledAsConstructor()) {
        context->throwError( QObject::trUtf8( "line %1: 'window' must be a constructor\n" ).arg( info.lineNumber()));
        return engine->undefinedValue();
    }

    // construct new window
    if ( context->argumentCount() >= 1 ) {
        itemPtr = new UiItem( UiItem::Window );

        // window( asset )
        if ( context->argument( 0 ).isString()) {
            itemPtr->setName( context->argument( 0 ).toString());
            itemPtr->setMaterial( context->argument( 0 ).toString());

            if ( context->argumentCount() >= 2 ) {
                // window( asset, parent )
                if ( context->argument( 1 ).isQObject()) {
                    itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 1 ).toQObject()));

                }
                // window( asset, posx, posy, sizex, sizey )
                // window( asset, posx, posy, sizex, sizey, parent )
                else if ( context->argument( 1 ).isNumber() && context->argumentCount() >= 5 ) {
                    if ( context->argument( 2 ).isNumber() &&
                         context->argument( 3 ).isNumber() &&
                         context->argument( 4 ).isNumber()) {

                        // set rect
                        itemPtr->setRect( context->argument( 1 ).toNumber(),
                                          context->argument( 2 ).toNumber(),
                                          context->argument( 3 ).toNumber(),
                                          context->argument( 4 ).toNumber()
                                          );

                        // set parent
                        if ( context->argumentCount() == 6 )
                            itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 5 ).toQObject()));
                    } else
                        goto fail;
                }
            }
        }
        // window( r, g, b, a )
        else if ( context->argument( 0 ).isNumber() && context->argumentCount() >= 4 ) {
            if ( context->argument( 1 ).isNumber() &&
                 context->argument( 2 ).isNumber() &&
                 context->argument( 3 ).isNumber()) {

                // set colour
                itemPtr->setBackColour( context->argument( 0 ).toNumber(),
                                        context->argument( 1 ).toNumber(),
                                        context->argument( 2 ).toNumber(),
                                        context->argument( 3 ).toNumber()
                                        );
                // set parent
                if ( context->argumentCount() >= 5 ) {
                    // window( r, g, b, a, parent )
                    if ( context->argument( 4 ).isQObject())
                        itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 4 ).toQObject()));
                    // window( r, g, b, a, x, y, w, h )
                    else if ( context->argument( 4 ).isNumber() && context->argumentCount() >= 8 ) {
                        if ( context->argument( 5 ).isNumber() &&
                             context->argument( 6 ).isNumber() &&
                             context->argument( 7 ).isNumber()) {

                            // set rect
                            itemPtr->setRect( context->argument( 4 ).toNumber(),
                                              context->argument( 5 ).toNumber(),
                                              context->argument( 6 ).toNumber(),
                                              context->argument( 7 ).toNumber()
                                              );

                            // window( r, g, b, a, x, y, w, h, parent )
                            if ( context->argumentCount() == 9 ) {
                                if ( context->argument( 8 ).isQObject())
                                    itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 8 ).toQObject()));
                            }
                        } else
                            goto fail;
                    } else
                        goto fail;
                }
            } else
                goto fail;

        } else if ( context->argument( 0 ).isQObject()) {
            // window( parent )
            if ( context->argument( 0 ).isQObject())
                itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 0 ).toQObject()));
        } else
            goto fail;
    }
    // window()
    else if ( context->argumentCount() == 0 )
        itemPtr = new UiItem( UiItem::Window );

    return engine->newQObject( itemPtr, QScriptEngine::QtOwnership );

fail:
    com.error( Sys_Common::SoftError,
               QObject::trUtf8( "line %1: invalid 'window' constructor\n" )
               .arg( info.lineNumber())
               );
    return engine->undefinedValue();
}

/*
===================
scriptItem
===================
*/
QScriptValue scriptItem( QScriptContext *context, QScriptEngine *engine ) {
    UiItem *itemPtr = NULL;
    QScriptContextInfo info( context );

    // must be a constructor
    if ( !context->isCalledAsConstructor()) {
        context->throwError( QObject::trUtf8( "line %1: 'container' must be a constructor\n" ).arg( info.lineNumber()));
        return engine->undefinedValue();
    }

    // construct item
    if ( context->argumentCount() >= 1 ) {
        // container( name )
        if ( context->argument( 0 ).isString()) {
            itemPtr = new UiItem( UiItem::Container, context->argument( 0 ).toString());
        }
        // container( parent )
        else if ( context->argument( 0 ).isQObject() && context->argumentCount() == 1 ) {
            itemPtr = new UiItem( UiItem::Container, "" );
            itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 0 ).toQObject()));
        } else
            goto fail;

        // container( name, parent )
        if ( context->argumentCount() >= 2 && !context->argument( 0 ).isQObject() && context->argument( 1 ).isQObject())
            itemPtr->setParent( qobject_cast<UiItem*>( context->argument( 1 ).toQObject()));
    }
    // container()
    else if ( context->argumentCount() == 0 )
        itemPtr = new UiItem( UiItem::Container );

    // all done
    return engine->newQObject( itemPtr, QScriptEngine::QtOwnership );

fail:
    com.error( Sys_Common::SoftError,
               QObject::trUtf8( "line %1: invalid 'container' constructor\n" )
               .arg( info.lineNumber())
               );
    return engine->undefinedValue();
}

/*
===================
construct
===================
*/
UI_ScriptEngine::UI_ScriptEngine() {
    QScriptEngine *e;

    // reset
    this->setInitialized( false );

    // init scripting engine
    this->mse = new Mod_ScriptEngine( "UiScipt", "scripts/", false, false );

    // add cmds
    cmd.add( "ui3_loadScript", loadUIScriptCmd, this->tr( "Loads User Interface script" ));
    cmd.add( "ui3_evaluateScript", evaluateUIScriptCmd, this->tr( "Evaluates User Interface script" ));

    // add custom funcs/constructors
    e = &mse->engine;
    e->globalObject().setProperty( "text", e->newFunction( scriptText ));
    e->globalObject().setProperty( "container", e->newFunction( scriptItem ));
    e->globalObject().setProperty( "window", e->newFunction( scriptWindow ));
    e->globalObject().setProperty( "UI", e->newQMetaObject( &m.staticMetaObject ));
    qRegisterMetaType<mtrHandle_t>( "mtrHandle_t" );

    // all done
    this->setInitialized();
}

/*
===================
destruct
===================
*/
UI_ScriptEngine::~UI_ScriptEngine() {
    if ( this->hasInitialized()) {
        this->setInitialized( false );

        // remove cmds
        cmd.remove( "ui3_loadScript" );
        cmd.remove( "ui3_evaluateScript" );

        // shut down scripting engine
        this->mse->~Mod_ScriptEngine();
    }
}
