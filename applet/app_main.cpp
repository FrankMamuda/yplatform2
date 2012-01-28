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
#include "app_main.h"
#include "../common/sys_common.h"
#include "../common/sys_cmd.h"
#include "../common/sys_cvar.h"
#include "../common/sys_filesystem.h"
#include "../common/sys_module.h"
#include "../gui/gui_main.h"

//
// classes
//
class App_Main m;

//
// commands
//
createSimpleCommand( m, shutdown )
#ifndef YP2_FINAL_RELEASE
createSimpleCommand( m, fatalError )
#endif

/*
================
shutdown
================
*/
void App_Main::shutdown() {
    // make sure we exit event loop
    this->timer->stop();

    // shut down subsystems
    if ( !com.hasCaughtError()) {
        com.gui()->shutdown();
        mod.shutdown();
        cmd.remove( "exit" );
#ifndef YP2_FINAL_RELEASE
        cmd.remove( "fatalError" );
#endif
        cv.saveConfig( Cvar::DefaultConfigFile );
        cmd.shutdown();
        pkg.shutdown();
        fs.shutdown();
        cv.shutdown();
    }

    QApplication::quit();
}

/*
================
parseArgs
================
*/
void App_Main::parseArgs( int argc, char *argv[] ) {
    int y;
    QList<QString> argList;
    QString cmdLine;

    // failsafe
    if ( argc <= 1 )
        return;

    // merge commandline
    for ( y = 1; y < argc; y++ ) {
        // this is dirrty, but we need to envelope string in quotes
        // since they are removed by commandline
        QString arg( argv[y] );
        if ( arg.contains( " " )) {
            arg.prepend( "\"" );
            arg.append( "\"" );
        }
        cmdLine.append( QString( "%1 " ).arg( arg ));
    }

    // failsafe
    if ( !cmdLine.startsWith( "+" )) {
        com.error( Sys_Common::SoftError, this->tr( "App_Main::parseArgs: arguments must start with '+'\n" ));
        return;
    }

    // compile a list of args
    argList = cmdLine.split( "+" );

    // nothing? - leave!
    if ( argList.isEmpty())
        return;

    // reset counter
    y = 0;
    foreach ( QString str, argList ) {
        if ( y != 0 )
            cmd.execute( str );
        y++;
    }
}

/*
================
update
================
*/
void App_Main::update() {
    if ( !com.hasCaughtError())
        mod.update();

    // dd: HACK to ensure autoscroll works (dirrty, I know)
    if ( com.milliseconds() < 2000 )
        com.gui()->autoScroll();
}

/*
================
cmdFatalError
================
*/
#ifndef YP2_FINAL_RELEASE
void App_Main::fatalError() {
    // for debugging (of gui in general)
    com.error( Sys_Common::FatalError, this->tr( "cmdFatalError: we have failed\n" ));
}
#endif

/*
================
startup
================
*/
int App_Main::startup( int argc, char *argv[] ) {
    int delay;
    QApplication app( argc, argv );
    Gui_Main gui;

    // damn decimal separators!
    // needed for EvaporationDevice calculator module, if it is ever backported
#ifndef Q_OS_WIN
    setlocale( LC_ALL, "C" );
#endif

    // i18n
    QTranslator translator;
    QString locale = QLocale::system().name();
    translator.load( QString( "i18n/YPlatform2_%1" ).arg( locale ));
    app.installTranslator( &translator );

    // create ui
    gui.show();
    com.setGui( &gui );

    // init subsystems
    cmd.init();
    cv.init();
    fs.init();
    mod.init();

    // parse config only AFTER filesystem has been initialized
    cv.parseConfig( Cvar::DefaultConfigFile );
    com.gui()->init();

#ifndef YP2_FINAL_RELEASE
    cmd.add( "fatalError", fatalErrorCmd, this->tr( "let's have some fish" ));
#endif
    cmd.add( "exit", shutdownCmd, this->tr( "shutdown platform" ));

    // set defaults for event handling & update subSystems
    delay = ( 1000 / Main::PlatformUpdateFrequency );
    this->timer = new QTimer( this );
    connect( this->timer, SIGNAL( timeout()), this, SLOT( update()));
    this->timer->start( delay );

    // print info
    com.gui()->printImage( ":/icons/settings", 24, 24 );
    com.print( this->tr( " ^5Platform: ^3initialization complete\n" ), 12 );
    com.gui()->printImage( ":/icons/about", 16, 16 );
    com.print( this->tr( "^5 Info: ^3to see full command list, execute \"^5cmd_list^3\", use ^5TAB^3 for completion\n" ));

    // check args after base system has init
    this->parseArgs( argc, argv );

    // exec application
    return app.exec();
}

/*
================
entry point
================
*/
int main( int argc, char *argv[] ) {
    return m.startup( argc, argv );
}
