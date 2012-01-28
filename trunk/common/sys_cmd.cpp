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
#include "sys_common.h"
#include "sys_cmd.h"
#include "sys_filesystem.h"
#include "sys_cvar.h"

//
// classes
//
class Sys_Cmd cmd;

//
// commands
//
createCommand( cmd, exec )
createCommand( cmd, echo )
createCommand( cmd, list )

/*
============
init
============
*/
void Sys_Cmd::init() {
    // add common commands
    this->add( "exec", execCmd, this->tr( "exec xml configuration file(s)" ));
    this->add( "cmd_list", listCmd, this->tr( "list all available commands" ));
    this->add( "echo", echoCmd, this->tr( "echo text to console" ));

    // we are initialized
    this->setInitialized();
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
    else
        this->setInitialized( false );

    // announce
    com.print( this->tr( "^2Sys_Cmd: ^5shutting down command subsystem\n" ));

    // remove all commands
    foreach ( pCmd *cmdFunc, this->cmdList )
        delete cmdFunc;
    this->cmdList.clear();
}

/*
============
add
============
*/
void Sys_Cmd::add( const QString &command, cmdCommand_t function, const QString &description  ) {
    // failsafe
    if ( this->find( command ) != NULL ) {
        com.print( this->tr( "^2Sys_Cmd::add: command ^3\"%1\" already exists\n" ).arg( command ));
        return;
    }

    // alloc new command
    this->cmdList << new pCmd( command, function, description );

    // add to completer
    com.gui()->addToCompleter( command );
}

/*
============
remove
============
*/
void Sys_Cmd::remove( const QString &command ) {
    pCmd *cmdPtr;

    cmdPtr = this->find( command );
    if ( cmdPtr != NULL ) {
        // remove from completer
        com.gui()->removeFromCompleter( cmdPtr->name());
        cmdList.removeOne( cmdPtr );
        delete cmdPtr;
    }
}

/*
============
echo
============
*/
void Sys_Cmd::echo( const QStringList &args ) {
    com.print( QString( "%1\n" ).arg( args.join( " " )));
}

/*
============
exec
============
*/
void Sys_Cmd::exec( const QStringList &args ) {
    if ( args.isEmpty()) {
        com.print( this->tr( "^3usage: ^2exec ^3[^2filenames^3] - execute xml configuration file(s)\n" ));
        return;
    }

    foreach ( QString filename, args ) {
        fs.defaultExtension( filename, ".xml" );
        cv.parseConfig( filename, true );
    }
}

/*
============
list
============
*/
void Sys_Cmd::list( const QStringList &args ) {
    // announce
    if ( !args.isEmpty()) {
        int numFiltered = 0;

        // get total filtered count
        foreach ( pCmd *cmdPtr, this->cmdList ) {
            if ( !args.isEmpty() && !cmdPtr->name().startsWith( args.first()))
                continue;
            numFiltered++;
        }
        if ( !numFiltered )
            com.print( this->tr( "^1Sys_Cmd::list: could not match any available commands\n" ));
        else
            com.print( this->tr( "^2Sys_Cmd::list: ^5matched ^3%1 ^5of ^3%2 ^5available commands:\n" ).arg( numFiltered ).arg( this->cmdList.count()));
    } else
        com.print( this->tr( "^2Sys_Cmdr::list: ^3%1 ^5available commands:\n" ).arg( this->cmdList.count()));

    foreach ( pCmd *cmdPtr, this->cmdList ) {
        if ( !args.isEmpty() && !cmdPtr->name().startsWith( args.first()))
            continue;

        if ( !cmdPtr->description().isEmpty() )
            com.print( QString( " ^3%1^5 - ^3%2\n" ).arg( cmdPtr->name(), cmdPtr->description()));
        else
            com.print( QString( " ^3%1\n" ).arg( cmdPtr->name()));
    }
}

/*
============
executeTokenized
============
*/
bool Sys_Cmd::executeTokenized( const QString &command, const QStringList &args ) {
    pCmd *cmdPtr;

    // find the command
    cmdPtr = this->find( command );
    if ( cmdPtr != NULL ) {
        // execute the function
        if ( cmdPtr->hasFunction()) {
            cmdPtr->execute( args );
            return true;
        }
    }

    // check cvars
    if ( cv.command( command, args ))
        return true;

    // report unknown command
    com.print( this->tr( "^2Sys_Cmd::execute: ^3unknown command \"%1\"\n" ).arg( command ));
    return false;
}

/*
============
find
============
*/
pCmd *Sys_Cmd::find( const QString &command ) const {
    foreach ( pCmd *cmdPtr, this->cmdList ) {
        if ( !QString::compare( command, cmdPtr->name(), Qt::CaseInsensitive ))
            return cmdPtr;
    }
    return NULL;
}

/*
============
execute
============
*/
bool Sys_Cmd::execute( const QString &buffer ) {
    int pos = 0;
    bool commandPending;
    QString cmd;
    QStringList args, lineSep;

    // separate multiline commands first
    lineSep = buffer.split( '\n' );

    // parse each line separately, split commands by semicolon
    foreach ( QString cmdString , lineSep ) {
        // reset
        commandPending = true;
        args.clear();
        cmd.clear();
        pos = 0;

        // tokenize the string and preserve quotes
        QRegExp rx( "((?:[^\\s\"]+)|(?:\"(?:\\\\\"|[^\"])*\"))" );
        while (( pos = rx.indexIn( cmdString, pos )) != -1 ) {
            if ( rx.cap( 1 ).at( 0 ) == QChar( ';' )) {
                commandPending = true;
                pos += rx.matchedLength();

                if ( !cmd.isEmpty()) {
                    return this->executeTokenized( cmd, args );
                    cmd.clear();
                    args.clear();
                }
                continue;
            }

            if ( commandPending ) {
                cmd = rx.cap( 1 );
                pos += rx.matchedLength();
                commandPending = false;
                continue;
            }

            args.append( rx.cap( 1 ));
            pos += rx.matchedLength();
        }

        // execute leftovers
        if ( !cmd.isEmpty())
            return this->executeTokenized( cmd, args );
    }
    return false;
}
