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

#ifndef SYS_CMD_H
#define SYS_CMD_H

//
// includes
//
#include "sys_shared.h"

//
// defines
//
typedef void ( *cmdCommand_t )( const QStringList &args );
Q_DECLARE_METATYPE( cmdCommand_t )

// command macros (wrappers)
#define createCommand( c, f ) static void f ## Cmd ( const QStringList &args ) { c.f( args ); }
#define createCommandPtr( c, f ) static void f ## Cmd ( const QStringList &args ) { c->f( args ); }
#define createSimpleCommand( c, f ) static void f ## Cmd ( const QStringList &args ) { Q_UNUSED( args ) c.f(); }
#define createSimpleCommandPtr( c, f ) static void f ## Cmd ( const QStringList &args ) { Q_UNUSED( args ) c->f(); }

//
// namespaces
//
namespace Cmd {
    class Sys_Cmd;
}

//
// class:pCmd
//
#ifndef MODULE_BUILD
class pCmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform console command" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( bool scripted READ isScripted WRITE setScripted )
    Q_PROPERTY( cmdCommand_t function READ function WRITE setFunction )
    Q_PROPERTY( QScriptValue scriptFunction READ scriptFunction WRITE setScriptFunction )
    Q_DISABLE_COPY( pCmd )

public:
    // constructor
    pCmd ( const QString &command, cmdCommand_t &function, const QString &description ) {
        this->setName( command );
        this->setFunction( function );
        this->setDescription( description );
        this->setScripted( false );
    }
    // constructor
    // should be pointer??
    pCmd ( const QString &command, QScriptValue &function, const QString &description ) {
        this->setName( command );
        this->setScriptFunction( function );
        this->setDescription( description );
        this->setScripted();
    }

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    cmdCommand_t function() const { return this->m_function; }
    QScriptValue scriptFunction() const { return this->m_scriptFunction; }
    bool isScripted() const { return this->m_scripted; }

    // other funcs
    void execute( const QStringList &args );
    bool hasFunction() const { if ( this->m_function != NULL ) return true; return false; }

public slots:
    // property setters
    void setName( const QString &command ) { this->m_name = command; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setFunction( const cmdCommand_t &function ) { this->m_function = function; }
    void setScriptFunction( const QScriptValue &function ) { this->m_scriptFunction = function; }
    void setScripted( bool scripted = true ) { this->m_scripted = scripted; }

private:
    // properties
    cmdCommand_t m_function;
    QScriptValue m_scriptFunction;
    QString m_name;
    QString m_description;
    bool m_scripted;
};

//
// class:Sys_Cmd
//
class Sys_Cmd : public QObject {
    Q_OBJECT
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_CLASSINFO( "description", "Command subsystem v3" )
    Q_ENUMS( Execution )

public:
    // execution modes
    enum Execution {
        Direct = 0,
        Delayed
    };
    void add( const QString &, cmdCommand_t, const QString & = QString::null );
    void add( const QString &, QScriptValue, const QString & = QString::null );
    void remove( const QString & );
    bool execute( const QString &, Execution = Direct );
    void executeDelayed();
    bool hasInitialized() const { return this->m_initialized; }
    QList<pCmd*> cmdList;
    pCmd *find( const QString & ) const;
    bool tokenize( const QString &string, QString &command, QStringList &arguments );

private:
    bool executeTokenized( const QString &, const QStringList & );
    bool m_initialized;
    QStringList delayedBuffer;

public slots:
    void init();
    void shutdown();
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }

    // commands
    void list( const QStringList & );
    void print( const QStringList & );
    void printImage( const QStringList & );
    void exec( const QStringList & );
};

//
// externals
//
extern class Sys_Cmd cmd;

#endif
#endif // SYS_CMD_H
