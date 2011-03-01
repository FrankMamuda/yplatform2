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

#ifndef SYS_CMD_H
#define SYS_CMD_H

//
// includes
//
#include "sys_shared.h"

//
// defines
//
typedef void ( *cmdCommand_t )();
class pCmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Console command" )
    Q_DISABLE_COPY( pCmd )

public:
    pCmd ( const QString &cmdName, cmdCommand_t function, const QString &description ) {
        this->name = cmdName;
        this->function = function;
        this->description = description;
    }
    QString name;
    QString description;
    cmdCommand_t    function;
};

//
// member function wrapper macro
//
#define createCommand( mClass, mFunc ) static void mFunc ## Cmd () { mClass.mFunc(); }
#define createCommandPtr( mClass, mFunc ) static void mFunc ## Cmd () { mClass->mFunc(); }

//
// class::Sys_Cmd
//
#ifndef MODULE_LIBRARY
class Sys_Cmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Command subsystem" )

public:
    void addCommand( const QString &cmdName, cmdCommand_t, const QString &description = QString::null );
    void removeCommand( const QString &cmdName );
    int argc();
    QString argv( int );
    bool execute( const QString &command );
    QList<pCmd*> cmdList;
    pCmd *find( const QString &name );

private:
    void tokenizeString( const QString &command );
    QStringList argumentList;
    bool initialized;

public slots:
    void init();
    void shutdown();
    void list();
    void exec();
    void echo();
};

#endif
#endif // SYS_CMD_H
