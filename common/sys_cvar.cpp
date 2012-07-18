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

//
// includes
//
#include <QtXml/QDomNode>
#include "sys_common.h"
#include "sys_cvar.h"
#include "sys_cmd.h"
#include "sys_module.h"
#include "sys_filesystem.h"
#include "../applet/app_main.h"
#include "../applet/app_hasher.h"
#include "../gui/gui_password.h"

//
// classes
//
Sys_Cvar cv;

//
// cvars
//
extern pCvar *fs_debug;
pCvar *sys_developer;
pCvar *sys_password;
pCvar *sys_protected;

//
// commands
//
createCommand( cv, set )
createCommand( cv, reset )
createCommand( cv, list )
createCommand( cv, create )

/*
============
init
============
*/
void Sys_Cvar::init() {
    // add commands
    cmd.add( "cv_set", setCmd, this->tr( "set console variable value" ));
    cmd.add( "cv_reset", resetCmd, this->tr( "reset console variable value to default" ));
    cmd.add( "cv_list", listCmd, this->tr( "list all registered console variables" ));
    cmd.add( "cv_create", createCmd, this->tr( "create a new console variable" ));

    // we are up and running
    this->setInitialized();

    // create cvar name validator
    QRegExp rx( "[A-z0-9_\\-]+" );
    this->validator = new QRegExpValidator( rx, this );

    // developer mode enabler
    sys_developer = cv.create( "sys_developer", "0", ( pCvar::Archive | pCvar::Password ));
    sys_password = cv.create( "sys_password", hash.encrypt( "" ), ( pCvar::Archive | pCvar::Password ));
    sys_protected = cv.create( "sys_protected", "0", ( pCvar::Archive | pCvar::Password ));
}

/*
============
shutdown
============
*/
void Sys_Cvar::shutdown() {
    // failsafe
    if ( !this->hasInitialized())
        return;

    // announce
    com.print( StrMsg + this->tr( "shutting down cvar subsystem\n" ));

    // remove commands
    cmd.remove( "cv_set" );
    cmd.remove( "cv_reset" );
    cmd.remove( "cv_list" );
    cmd.remove( "cv_create" );

    // remove validator
    delete this->validator;

    // cleanup
    this->clear();
}

/*
============
validate
============
*/
bool Sys_Cvar::validate( const QString &s ) const {
    int pos;

    // check for illegal chars
    if ( s.isNull())
        return false;

    // validate cvar name
    pos = 0;
    QString str( s );
    QRegExpValidator::State state = this->validator->validate( str, pos );

    // is valid string?
    if ( state == QRegExpValidator::Acceptable )
        return true;

    // fail
    return false;
}

/*
============
find
============
*/
pCvar *Sys_Cvar::find( const QString &name ) const {
    if ( !this->validate( name )) {
        com.error( StrSoftError + this->tr( "invalid cvar name \"%1\"\n" ).arg( name ));
        return NULL;
    }

    foreach ( pCvar *cvarPtr, this->cvarList ) {
        if ( !QString::compare( name, cvarPtr->name(), Qt::CaseInsensitive ))
            return cvarPtr;
    }
    return NULL;
}

/*
============
clear
============
*/
void Sys_Cvar::clear() {
    foreach ( pCvar *cvarPtr, this->cvarList )
        delete cvarPtr;
}

/*
============
set
============
*/
void Sys_Cvar::set( const QStringList &args ) {
    pCvar *cvarPtr;

    // check args
    if ( args.count() < 2 ) {
        com.print( Sys::cYellow + this->tr( "usage: ^2cv_set ^3[^2variable^3] [^2value^3] (^5flags^3)\n" ));
        return;
    }

    // find cvar
    cvarPtr = this->find( args.first());

    // set value (and flags if any)
    if ( cvarPtr != NULL ) {
        cvarPtr->set( args.at( 1 ));

        if ( args.count() == 3 )
            cvarPtr->flags |= static_cast<pCvar::Flags>( args.at( 2 ).toInt());
    } else
        com.print( StrWarn + this->tr( "cvar \"%1\" does not exist, use ^2cv_create ^3to create one\n" ).arg( args.first()));
}

/*
============
reset
============
*/
void Sys_Cvar::reset( const QStringList &args ) {
    pCvar   *cvarPtr;

    if ( args.isEmpty()) {
        com.print( Sys::cYellow + this->tr( "usage: ^2cv_reset ^3[^2variable^3]\n" ));
        return;
    }
    cvarPtr = this->find( args.first());

    if ( cvarPtr != NULL ) {
        cvarPtr->reset();
    } else
        com.error( StrSoftError + this->tr( "could not find cvar \"%1\"\n" ).arg( args.first()));
}

/*
============
create
============
*/
void Sys_Cvar::create( const QStringList &args ) {
    // check args
    if ( args.count() < 2 ) {
        com.print( Sys::cYellow + this->tr( "usage: ^2cv_create ^3[^2variable^3] [^2value^3] (^5flags^3)\n" ));
        return;
    }

    // find cvar, create one if non-existant
    if ( this->find( args.first()) == NULL ) {
        if ( args.count() == 3 )
            this->create( args.first(), args.at( 1 ), static_cast<pCvar::Flags>( args.at( 2 ).toInt()));
        else
            this->create( args.first(), args.at( 1 ));
    }
}

/*
============
list
============
*/
void Sys_Cvar::list( const QStringList &args ) {
    // announce
    if ( !args.isEmpty()) {
        int numFiltered = 0;

        // get total filtered count
        foreach ( pCvar *cvarPtr, this->cvarList ) {
            if ( !args.isEmpty() && !cvarPtr->name().startsWith( args.first()))
                continue;
            numFiltered++;
        }

        if ( !numFiltered )
            com.print( StrWarn + this->tr( "could not match any available cvars\n" ));
        else
            com.print( StrMsg + this->tr( "matched ^3%1 ^5of ^3%2 ^5available cvars:\n" ).arg( numFiltered ).arg( this->cvarList.count()));
    } else
        com.print( StrMsg + this->tr( "^3%1 ^5available cvars:\n" ).arg( this->cvarList.count()));

    foreach ( pCvar *cvarPtr, this->cvarList ) {
        if ( !args.isEmpty() && !cvarPtr->name().startsWith( args.first()))
            continue;

        com.print( QString( "  ^5'%1': ^2'%2' " ).arg( cvarPtr->name(), cvarPtr->string()));

        // append flags
        if ( cvarPtr->flags.testFlag( pCvar::ReadOnly ))
            com.print( "^3R" );

        if ( cvarPtr->flags.testFlag( pCvar::Archive ))
            com.print( "^3A" );

        if ( cvarPtr->flags.testFlag( pCvar::Latched ))
            com.print( "^3L" );

        if ( cvarPtr->flags.testFlag( pCvar::Password ))
            com.print( "^3P" );

        com.print( "\n" );
    }
}

/*
============
create
============
*/
pCvar *Sys_Cvar::create( const QString &name, const QString &string, pCvar::Flags flags, const QString &description, bool mCvar ) {
    if ( !this->validate( name )) {
        com.error( StrSoftError + this->tr( "invalid cvar name \"%1\"\n" ).arg( name ));
        return NULL;
    }

    // search for available cvars
    pCvar *cvarPtr = this->find( name );

    // when cvar is created from configuration file, it looses mCvar flag
    // make sure we connect it for updates
    if ( cvarPtr != NULL && mCvar )
        this->connect( cvarPtr, SIGNAL( valueChanged( QString, QString )), &mod, SLOT( updateCvar( QString, QString )));

    // create new
    if ( cvarPtr == NULL ) {
        // allocate & add to list
        cvarPtr = new pCvar( name, string, flags, description, mCvar );
        this->cvarList << cvarPtr;
        com.gui()->addToCompleter( cvarPtr->name());
    }
    return cvarPtr;
}

/*
============
command
============
*/
bool Sys_Cvar::command( const QString &name, const QStringList &args ) {
    pCvar  *cvarPtr;

    // find cvar
    cvarPtr = this->find( name );
    if ( !cvarPtr )
        return false;

    // print out current value if no args, otherwise set new value
    if ( args.isEmpty()) {
        com.print( Sys::cYellow + this->tr( " \"%1\" ^5is ^3\"%2\"^5, default: ^3\"%3\"\n" ).arg(
                      cvarPtr->name(),
                      cvarPtr->string(),
                      cvarPtr->resetString()));

        if ( !cvarPtr->latchString().isEmpty())
            com.print( Sys::cYellow + this->tr( " latched: \"%1\"\n" ).arg( cvarPtr->latchString()));
    } else
        cvarPtr->set( args.first());

    // all ok
    return true;
}

/*
===============
parseConfig
===============
*/
void Sys_Cvar::parseConfig( const QString &filename, bool verbose ) {
    QDomDocument configFile;

    // read buffer
    QByteArray buffer = fs.readFile( filename, Sys_Filesystem::Silent );

    // failsafe
    if ( buffer.isNull()) {
        if ( !QString::compare( filename, Cvar::DefaultConfigFile )) {
            com.print( StrWarn + this->tr( "configuration file does not exist, creating \"%1\"\n" ).arg( filename ));
            fs.touch( filename, Sys_Filesystem::Silent );
        }

        if ( verbose )
            com.error( StrSoftError + this->tr( "configuration file \"%1\" does not exist\n" ).arg( filename ));

        return;
    }

    // failsafe
    if ( buffer.isEmpty()) {
        if ( verbose )
            com.error( StrSoftError + this->tr( "configuration file \"%1\" is empty\n" ).arg( filename ));

        return;
    }

    //
    // parse document
    //
    configFile.setContent( buffer );
    QDomNode configNode = configFile.firstChild();
    while ( !configNode.isNull()) {
        if ( configNode.isElement()) {
            QDomElement configElement = configNode.toElement();

            // check element name
            if ( QString::compare( configElement.tagName(), "config" )) {
                com.error( StrSoftError + this->tr( "expected <config> in \"%1\"\n" ).arg( filename ));
                return;
            }

            QDomNode cvarNode = configElement.firstChild();
            while ( !cvarNode.isNull()) {
                if ( cvarNode.isElement()) {
                    QDomElement cvarElement = cvarNode.toElement();

                    // check element name
                    if ( !QString::compare( cvarElement.tagName(), "cvar" )) {
                        // check cvar name
                        if ( !cvarElement.hasAttribute( "name" )) {
                            com.error( StrSoftError + this->tr( "parseConfig: nameless <cvar> in \"%1\"\n" ).arg( filename ));
                            return;
                        }

                        // now find the cvar
                        QString cvarName = cvarElement.attribute( "name" );
                        pCvar *cvarPtr = this->find( cvarName );

                        // if it doesn't exist - create it, if it already exists - force the stored value
                        if ( cvarPtr != NULL ) {
                            cvarPtr->set( cvarElement.text(), static_cast<pCvar::AccessFlags>( pCvar::Config | pCvar::Force ));

                            if ( verbose )
                                com.print( StrWarn + this->tr( "setting cvar \"%1\" value \"%2\"\n" ).arg( cvarName, cvarElement.text()));

                        } else {
                            this->create( cvarName, cvarElement.text(), ( pCvar::Flags )cvarElement.attribute( "flags" ).toInt());

                            if ( verbose )
                                com.print( StrWarn + this->tr( "creating cvar \"%1\" with value \"%2\"\n" ).arg( cvarName, cvarElement.text()));
                        }
                    } else if ( !QString::compare( cvarElement.tagName(), "cmd" )) {
                        // check cvar name
                        if ( !cvarElement.hasAttribute( "name" )) {
                            com.error( StrSoftError + this->tr( "nameless <cmd> in \"%1\"\n" ).arg( filename ));
                            return;
                        }

                        // now find the cvar
                        QString cmdName = cvarElement.attribute( "name" );
                        foreach ( pCmd *cmdPtr, cmd.cmdList ) {
                            if ( !QString::compare( cmdName, cmdPtr->name())) {
                                cmd.execute( cmdName + " " + cvarElement.text());
                                break;
                            }
                        }
                    } else {
                        com.error( StrSoftError + this->tr( "expected <cvar> or <cmd> in \"%1\"\n" ).arg( filename ));
                        return;
                    }
                }
                cvarNode = cvarNode.nextSibling();
            }
            break;
        }
        configNode = configNode.nextSibling();
    }

    // clear buffer
    buffer.clear();
}

/*
===============
saveConfig
===============
*/
void Sys_Cvar::saveConfig( const QString &filename ) {
    // create document
    QDomDocument configFile;

    // create config tag
    QDomElement configElement = configFile.createElement( "config" );
    configElement.setAttribute( "version", "1.1" );
    configElement.setAttribute( "generator", "YPlatform2" );
    configFile.appendChild( configElement );

    // generate cvars config strings
    foreach ( pCvar *cvarPtr, this->cvarList ) {
        if ( cvarPtr->flags.testFlag( pCvar::Archive )) {
            if ( fs_debug->integer())
                com.print( StrDebug + this->tr( "setting \"%1\" value \"%2\"\n" ).arg( cvarPtr->name(), cvarPtr->string()));

            QDomElement cvarElement = configFile.createElement( "cvar" );
            cvarElement.setAttribute( "name", cvarPtr->name());
            cvarElement.setAttribute( "flags", cvarPtr->flags );
            configElement.appendChild( cvarElement );

            QDomText cvarText = configFile.createTextNode( cvarPtr->string());
            cvarElement.appendChild( cvarText );
        }
    }

    // write out
    fileHandle_t fileOut;
    if ( fs.open( pFile::Write, filename, fileOut, Sys_Filesystem::Silent ) != -1 ) {
        fs.print( fileOut, configFile.toString());
        fs.close( fileOut );
    }
}
