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
// dd, cvar subsystem complete rewrite as part of YCommons 2.9
//

//
// includes
//
#include <QtXml/QDomNode>
#include "sys_common.h"
#include "sys_cvar.h"
#include "sys_cmd.h"
#include "sys_filesystem.h"
#include "../applet/app_main.h"

//
// classes
//
extern class Sys_Common com;
extern class Sys_Common com;
extern class Sys_Filesystem fs;
extern class Sys_Cmd cmd;
Sys_Cvar cv;

//
// cvars
//
extern pCvar *fs_debug;

//
// commands
//
createCommand( cv, set )
createCommand( cv, reset )
createCommand( cv, list )
createCommand( cv, create )

/*
============
validate
============
*/
bool Sys_Cvar::validate( const QString &s ) {
    // check for illegal chars
    if ( s.isNull())
        return false;

    if ( s.contains( '\\' ))
        return false;

    if ( s.contains( '\"' ))
        return false;

    if ( s.contains( ';' ))
        return false;

    return true;
}

/*
============
find
============
*/
pCvar *Sys_Cvar::find( const QString &name ) {
    if ( !this->validate( name )) {
        com.error( ERR_SOFT, this->tr( "Sys_Cvar::find: invalid name\n" ));
        return NULL;
    }

    foreach ( pCvar *cvarPtr, this->cvars ) {
        if ( !QString::compare( name, cvarPtr->name ))
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
    foreach ( pCvar *cvarPtr, this->cvars )
        delete cvarPtr;
}

/*
============
shutdown
============
*/
void Sys_Cvar::shutdown() {
    // failsafe
    if ( !this->initialized )
        return;

    // announce
    com.print( this->tr( "^2Sys_Cvar: ^5shutting down cvar subsystem\n" ));

    // remove commands
    cmd.removeCommand( "cv_set" );
    cmd.removeCommand( "cv_reset" );
    cmd.removeCommand( "cv_list" );
    cmd.removeCommand( "cv_create" );

    // cleanup
    this->clear();
}

/*
============
reset
============
*/
void Sys_Cvar::reset() {
    int     args;
    pCvar   *cvarPtr;

    args = cmd.argc();

    if ( args < 2 ) {
        com.print( this->tr( "^3usage: cv_reset [variable] [value]\n" ));
        return;
    }
    cvarPtr = this->find( cmd.argv(1));

    if ( cvarPtr != NULL ) {
        cvarPtr->set( cvarPtr->reset, false );
    } else
        com.error( ERR_SOFT, this->tr( "Sys_Cvar::reset: could not find cvar \"%1\"\n" ).arg( cmd.argv( 1 )));
}

/*
============
set
============
*/
void Sys_Cvar::set() {
    int     numArgs;
    QString string( "" );
    QString name;
    int     y;
    pCvar   *cvarPtr;
    int flags = 0;

    numArgs = cmd.argc();
    qDebug() << cmd.argv(0) << cmd.argv(1) << cmd.argv(2);
    if ( numArgs < 3 ) {
        com.print( this->tr( "^3usage: cv_set [variable] [value] (flags)\n" ));
        return;
    }
    name = cmd.argv(1);
    qDebug() << cmd.argv(1) << cmd.argv(2);
    cvarPtr = this->find( name );

    for ( y = 2; y < numArgs; y++ ) {
        string.append( cmd.argv(y));

        if ( y < numArgs - 1 )
            string.append( " " );
    }

    if ( numArgs == 4 )
        flags = cmd.argv(3).toInt();

    if ( cvarPtr != NULL ) {
        cvarPtr->set( string );
        cvarPtr->flags |= flags;
    } else
        com.error( ERR_SOFT, this->tr( "Sys_Cvar::set: cvar \"%1\" does not exist, try creating one\n" ).arg( name ));
}

/*
============
create
============
*/
void Sys_Cvar::create() {
    int     numArgs;
    int     flags = 0;
    pCvar   *cvar;

    numArgs = cmd.argc();

    if ( numArgs < 3 ) {
        com.print( "^3" + this->tr( "^3usage: cv_create [variable] [value] (flags)\n" ));
        return;
    }
    cvar = this->find( cmd.argv(1));
    if ( numArgs == 4 ) {
        flags = cmd.argv(3).toInt();

        if ( flags < 0 /*|| flags > CVAR_PASSWORD*/ ) {
            com.error( ERR_SOFT, this->tr( "Sys_Cvar::create: invalid flags\n" ));
            return;
        }
    }

    if ( cvar == NULL )
        cvar = this->create( cmd.argv(1), cmd.argv(2), flags );
}

/*
============
init
============
*/
void Sys_Cvar::init() {
    // add commands
    cmd.addCommand( "cv_set", setCmd, this->tr( "set console variable value" ));
    cmd.addCommand( "cv_reset", resetCmd, this->tr( "reset console variable value to default" ));
    cmd.addCommand( "cv_list", listCmd, this->tr( "list all registered console variables" ));
    cmd.addCommand( "cv_create", createCmd, this->tr( "create a new console variable" ));

    // we are up and running
    this->initialized = true;
}

/*
============
create
============
*/
pCvar *Sys_Cvar::create( const QString &name, const QString &string, int flags, const QString &description, bool mCvar ) {
    if ( !this->validate( name )) {
        com.error( ERR_SOFT, this->tr( "Sys_Cvar::create: invalid name\n" ));
        return NULL;
    }

    // search for available cvars
    pCvar *cvarPtr = this->find( name );

    // create new
    if ( cvarPtr == NULL ) {
        // allocate & add to list
        cvarPtr = new pCvar( name, string, flags, description, mCvar );
        this->cvars << cvarPtr;
        com.gui->addToCompleter( cvarPtr->name );
    }
    return cvarPtr;
}

/*
============
list
============
*/
void Sys_Cvar::list() {
    bool match = false;

    if ( cmd.argc() > 1 )
        match = true;

    // announce
    com.print( this->tr( "^2Sys_Cvar::list: ^5registered ^3%1 ^5cvars:\n" ).arg( this->cvars.count()));
    foreach ( pCvar *cvarPtr, this->cvars ) {
        if ( match && !cvarPtr->name.startsWith( cmd.argv( 1 )))
            continue;

        com.print( QString( "  ^5'%1': ^2'%2' " ).arg( cvarPtr->name, cvarPtr->stringValue ));

        // append flags
        if ( cvarPtr->flags & CVAR_ROM )
            com.print( "^3R" );

        if ( cvarPtr->flags & CVAR_ARCHIVE )
            com.print( "^3A" );

        if ( cvarPtr->flags & CVAR_LATCH )
            com.print( "^3L" );
#if 0
        if ( cvarPtr->flags & CVAR_PASSWORD )
            com.print( "^3P" );
#endif
        com.print( "\n" );
    }
}

/*
============
command
============
*/
bool Sys_Cvar::command() {
    pCvar  *cvarPtr;

    // check variables
    cvarPtr = this->find( cmd.argv(0));

    if ( !cvarPtr )
        return false;

    // perform a variable print or set
    if ( cmd.argc() == 1 ) {
        com.print( this->tr( " ^3\"%1\" ^5is ^3\"%2\"^5, default: ^3\"%3\"\n" ).arg(
                      cvarPtr->name,
                      cvarPtr->stringValue,
                      cvarPtr->reset ));

        if ( !cvarPtr->latch.isEmpty())
            com.print( this->tr( " ^3latched: \"%1\"\n" ).arg( cvarPtr->latch ));

        return true;
    }

    // set the value if forcing is not required
    cvarPtr->set( cmd.argv(1), false );
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
    byte *buffer;
    int len = fs.readFile( filename, &buffer, FS_FLAGS_SILENT );

    // failsafe
    if ( len == -1 ) {
        if ( !QString::compare( filename, DEFAULT_CONFIG_FILE )) {
            com.print( this->tr( "^2Sys_Cvar::parseConfig: ^3configuration file does not exist, creating \"%1\"\n" ).arg( filename ));
            fs.touch( filename, FS_FLAGS_SILENT );
        }

        if ( verbose )
            com.error( ERR_SOFT, this->tr( "Sys_Cvar::parseConfig: configuration file \"%1\" does not exist\n" ).arg( filename ));

        return;
    }

    // failsafe
    if ( len == 0 ) {
        if ( verbose )
            com.error( ERR_SOFT, this->tr( "Sys_Cvar::parseConfig: configuration file \"%1\" is empty\n" ).arg( filename ));

        return;
    }

    //
    // parse document
    //
    configFile.setContent( QByteArray(( const char* )buffer, len ));
    QDomNode configNode = configFile.firstChild();
    while ( !configNode.isNull()) {
        if ( configNode.isElement()) {
            QDomElement configElement = configNode.toElement();

            // check element name
            if ( QString::compare( configElement.tagName(), "config" )) {
                com.error( ERR_SOFT, this->tr( "Sys_Cvar::parseConfig: expected <config> in \"%1\"\n" ).arg( filename ));
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
                            com.error( ERR_SOFT, this->tr( "Sys_Cvar::parseConfig: nameless <cvar> in \"%1\"\n" ).arg( filename ));
                            return;
                        }

                        // now find the cvar
                        QString cvarName = cvarElement.attribute( "name" );
                        pCvar *cvarPtr = this->find( cvarName );

                        // if it doesn't exist - create it, if it already exists - force the stored value
                        if ( cvarPtr != NULL ) {
                            cvarPtr->set( cvarElement.text(), true );

                            if ( verbose )
                                com.print( this->tr( "^2Sys_Cvar::parseConfig: ^3setting cvar \"%1\" value \"%2\"\n" ).arg( cvarName, cvarElement.text()));

                        } else {
                            this->create( cvarName, cvarElement.text(), cvarElement.attribute( "flags" ).toInt());

                            if ( verbose )
                                com.print( this->tr( "^2Sys_Cvar::parseConfig: ^3creating cvar \"%1\" with value \"%2\"\n" ).arg( cvarName, cvarElement.text()));
                        }
                    } else if ( !QString::compare( cvarElement.tagName(), "cmd" )) {
                        // check cvar name
                        if ( !cvarElement.hasAttribute( "name" )) {
                            com.error( ERR_SOFT, this->tr( "Sys_Cvar::parseConfig: nameless <cmd> in \"%1\"\n" ).arg( filename ));
                            return;
                        }

                        // now find the cvar
                        QString cmdName = cvarElement.attribute( "name" );
                        foreach ( pCmd *cmdPtr, cmd.cmdList ) {
                            if ( !QString::compare( cmdName, cmdPtr->name )) {
                                cmd.execute( cmdName + " " + cvarElement.text());
                                break;
                            }
                        }
                    } else {
                        com.error( ERR_SOFT, this->tr( "Sys_Cvar::parseConfig: expected <cvar> or <cmd> in \"%1\"\n" ).arg( filename ));
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
    fs.freeFile( filename );
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
    foreach ( pCvar *cvarPtr, this->cvars ) {
        if ( cvarPtr->flags & CVAR_ARCHIVE ) {
            if ( fs_debug->integer())
                com.print( this->tr( "^6Sys_Cvar::saveConfig: setting \"%1\" value \"%2\"\n" ).arg( cvarPtr->name, cvarPtr->stringValue ));

            QDomElement cvarElement = configFile.createElement( "cvar" );
            cvarElement.setAttribute( "name", cvarPtr->name );
            cvarElement.setAttribute( "flags", cvarPtr->flags );
            configElement.appendChild( cvarElement );

            QDomText cvarText = configFile.createTextNode( cvarPtr->stringValue );
            cvarElement.appendChild( cvarText );
        }
    }

    // write out
    fileHandle_t fileOut;
    if ( fs.fOpenFile( FS_MODE_WRITE, filename, fileOut, FS_FLAGS_SILENT ) != -1 ) {
        fs.fPrint( fileOut, configFile.toString());
        fs.fCloseFile( fileOut );
    }
}
