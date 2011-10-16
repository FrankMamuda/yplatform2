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
// the new command subsystem
//

//
// includes
//
#include "sys_cmd.h"
#include "sys_common.h"
#include "sys_cvar.h"
#include "sys_filesystem.h"

//
// classes
//
class Sys_Cmd cmd;

//
// commands
//
createCommand( cmd, exec )
createCommand( cmd, list )
createCommand( cmd, echo )

/*
============
addCommand
============
*/
void Sys_Cmd::add( const QString &cmdName, cmdCommand_t function, const QString &description ) {
    pCmd *cmdPtr;

    // failsafe
    if ( this->find( cmdName ) != NULL ) {
        com.print( this->tr( "^2Sys_Cmd::add: ^3\"%1\" already defined\n" ).arg( cmdName ));
        return;
    }

    // alloc new command
    cmdPtr = new pCmd( cmdName, function, description );
    cmdList << cmdPtr;

    // add to completer
    com.gui()->addToCompleter( cmdPtr->name() );
}

/*
============
removeCommand
============
*/
void Sys_Cmd::remove( const QString &cmdName ) {
    pCmd *cmdPtr;

    cmdPtr = this->find( cmdName );
    if ( cmdPtr != NULL ) {
        // remove from completer
        com.gui()->removeFromCompleter( cmdPtr->name());
        cmdList.removeOne( cmdPtr );
        delete cmdPtr;
    }
}

/*
============
tokenize
============
*/
void Sys_Cmd::tokenize( const QString &command ) {
    QString token;
    bool inQuotes = false;

    // clean up
    this->argumentList.clear();

    if ( !command.length())
        return;

    // separate strings
    foreach ( QChar ch, command ) {
        if ( ch == QChar( '\"' ) && !inQuotes ) {
            inQuotes = true;
            continue;
        } else if ( ch == QChar( '\"' ) && inQuotes ) {
            inQuotes = false;
            continue;
        }

        if ( ch == QChar( ' ' ) && !inQuotes && token.length()) {
            this->argumentList << token;
            token.clear();
        } else {
            if ( ch == QChar( ' ' ) && !inQuotes )
                continue;

            token.append( ch );
        }
    }

    // leftover
    if ( !token.isEmpty())
        this->argumentList << token;
}

/*
============
argc
============
*/
int Sys_Cmd::argc() const {
    return this->argumentList.count();
}

/*
============
argv
============
*/
QString Sys_Cmd::argv( unsigned int arg ) const {
    if ( arg >= static_cast<unsigned int>( this->argumentList.count()))
        return QString( "" );

    return this->argumentList.at( arg );
}

/*
============
execute
============
*/
bool Sys_Cmd::execute( const QString &command ) {
    pCmd *cmdPtr;

    // execute the command line
    this->tokenize( command );

    if ( !this->argc())
        return false;

    // search for the command
    cmdPtr = this->find( this->argumentList.first());
    if ( cmdPtr != NULL ) {
        // execute the function
        if ( cmdPtr->hasFunction() )
            cmdPtr->exec();

        return true;
    }

    // check cvars
    if ( cv.command())
        return true;

    // unregistered command
    com.print( this->tr( "^2Sys_Cmd::execute: ^3unknown command \"%1\"\n" ).arg( this->argumentList.first()));
    return false;
}

/*
============
cmdEcho
============
*/
void Sys_Cmd::echo() {
    int	y;

    for ( y = 1; y < this->argc(); y++ )
        com.print( QString( "%1 " ).arg( this->argv( y )));

    com.print( "\n" );
}

/*
============
list
============
*/
void Sys_Cmd::list() {
    bool match = false;

    if ( this->argc() > 1 )
        match = true;

    com.print( this->tr( "^2Sys_Cmd::list: ^5registered ^3%1 ^5commands:\n" ).arg( this->cmdList.count()));
    foreach ( pCmd *cmdPtr, this->cmdList ) {
        if ( match && !cmdPtr->name().startsWith( this->argv( 1 )))
            continue;

        if ( !cmdPtr->description().isEmpty() )
            com.print( QString( " ^3%1^5 - ^3%2\n" ).arg( cmdPtr->name(), cmdPtr->description()));
        else
            com.print( QString( " ^3%1\n" ).arg( cmdPtr->name()));
    }
}


/*
============
exec
============
*/
void Sys_Cmd::exec() {
    QString filename;

    if ( this->argc() != 2 ) {
        com.print( "^3" + this->tr( "usage: cmd_exec [filename] - execute xml configuration file\n" ));
        return;
    }

    filename = this->argv( 1 );
    fs.defaultExtension( filename, ".xml" );
    cv.parseConfig( filename, true );
}

/*
============
init
============
*/
void Sys_Cmd::init() {
    // add commands
    this->add( "cmd_exec", execCmd, this->tr( "exec xml configuration file" ));
    this->add( "cmd_list", listCmd, this->tr( "list all available commands" ));
    this->add( "echo", echoCmd, this->tr( "echo text to console" ));

    // we are initialized
    this->setInitialized();;
}

/*
============
shutdown
============
*/
void Sys_Cmd::shutdown() {
    // failsafe
    if ( !this->hasInitialized())
        return;

    // announce
    com.print( this->tr( "^2Sys_Cmd: ^5shutting down command subsystem\n" ));

    // remove ALL commands
    foreach ( pCmd *cmdFunc, this->cmdList )
        delete cmdFunc;

    this->cmdList.clear();
}

/*
============
find
============
*/
pCmd *Sys_Cmd::find( const QString &name ) const {
    foreach ( pCmd *cmdPtr, this->cmdList ) {
        if ( !QString::compare( name, cmdPtr->name(), Qt::CaseInsensitive ))
            return cmdPtr;
    }
    return NULL;
}