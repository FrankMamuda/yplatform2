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

//
// namespaces
//
namespace Cmd {
    class Sys_Cmd;
}

//
// class:pCmd
//
class pCmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform console command" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( cmdCommand_t function READ function WRITE setFunction )
    Q_DISABLE_COPY( pCmd )

public:
    // constructor
    pCmd ( const QString &cmdName, cmdCommand_t &function, const QString &description ) {
        this->setName( cmdName );
        this->setFunction( function );
        this->setDescription( description );
    }

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    cmdCommand_t function() const { return this->m_function; }

    // other funcs
    void exec() { this->m_function(); }
    bool hasFunction() const { if ( this->m_function ) return true; return false; }

public slots:
    // property setters
    void setName( const QString &cmdName ) { this->m_name = cmdName; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setFunction( const cmdCommand_t &function ) { this->m_function = function; }

private:
    // properties
    cmdCommand_t m_function;
    QString m_name;
    QString m_description;
};

//
// member function wrapper macro
//
#define createCommand( mClass, mFunc ) static void mFunc ## Cmd () { mClass.mFunc(); }
#define createCommandPtr( mClass, mFunc ) static void mFunc ## Cmd () { mClass->mFunc(); }

//
// class:Sys_Cmd
//
#ifndef MODULE_LIBRARY
class Sys_Cmd : public QObject {
    Q_OBJECT
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_CLASSINFO( "description", "Command subsystem" )

public:
    void add( const QString &cmdName, cmdCommand_t, const QString &description = QString::null );
    void remove( const QString &cmdName );
    int argc() const;
    QString argv( unsigned int ) const;
    QList<pCmd*> cmdList;
    pCmd *find( const QString &name ) const;
    bool execute( const QString &command );
    bool hasInitialized() const { return this->m_initialized; }

private:
    void tokenize( const QString &command );
    bool m_initialized;
    QStringList argumentList;

public slots:
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void init();
    void shutdown();
    void list();
    void exec();
    void echo();
};

//
// externals
//
#ifndef MODULE_LIBRARY
extern class Sys_Cmd cmd;
#endif

#endif
#endif // SYS_CMD_H
