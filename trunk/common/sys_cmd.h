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
typedef struct cmdFunction_s {
    QString         name;
    cmdCommand_t    function;
    QString         description;
} cmdFunction_t;

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

public:
    void addCommand( const QString &cmdName, cmdCommand_t, const QString &description = QString::null );
    void removeCommand( const QString &cmdName );
    int argc();
    QString argv( int );
    bool execute( const QString &command );
    QList<cmdFunction_t*> cmdList;
    cmdFunction_t *find( const QString &name );

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
