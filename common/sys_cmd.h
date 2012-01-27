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
#ifndef MODULE_LIBRARY
class pCmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform console command" )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString description READ description WRITE setDescription )
    Q_PROPERTY( cmdCommand_t function READ function WRITE setFunction )
    Q_DISABLE_COPY( pCmd )

public:
    // constructor
    pCmd ( const QString &command, cmdCommand_t &function, const QString &description  ) {
        this->setName( command );
        this->setFunction( function );
        this->setDescription( description );
    }

    // property getters
    QString name() const { return this->m_name; }
    QString description() const { return this->m_description; }
    cmdCommand_t function() const { return this->m_function; }

    // other funcs
    void execute( const QStringList &args ) { this->m_function( args ); }
    bool hasFunction() const { if ( this->m_function != NULL ) return true; return false; }

public slots:
    // property setters
    void setName( const QString &command ) { this->m_name = command; }
    void setDescription( const QString &description ) { this->m_description = description; }
    void setFunction( const cmdCommand_t &function ) { this->m_function = function; }

private:
    // properties
    cmdCommand_t m_function;
    QString m_name;
    QString m_description;
};

//
// class:Sys_Cmd
//
class Sys_Cmd : public QObject {
    Q_OBJECT
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_CLASSINFO( "description", "Command subsystem v3" )

public:
    void add( const QString &, cmdCommand_t, const QString & = QString::null );
    void remove( const QString & );
    bool execute( const QString & );
    bool hasInitialized() const { return this->m_initialized; }
    QList<pCmd*> cmdList;
    pCmd *find( const QString & ) const;

private:
    bool executeTokenized( const QString &, const QStringList & );
    bool m_initialized;

public slots:
    void init();
    void shutdown();
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }

    // commands
    void list( const QStringList & );
    void echo( const QStringList & );
    void exec( const QStringList & );
};

//
// externals
//
extern class Sys_Cmd cmd;

#endif
#endif // SYS_CMD_H
