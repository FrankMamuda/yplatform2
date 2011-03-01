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
// defines
//
class App_Main m;
extern class Sys_Common com;
extern class Sys_Cmd cmd;
extern class Sys_Cvar cv;
extern class Sys_Filesystem fs;
extern class Sys_Module mod;

//
// externals
//
extern void cmdParseManifest();

//
// commands
//
createCommand( m, shutdown )
#ifndef YP2_FINAL_RELEASE
createCommand( m, fatalError )
#endif

/*
================
shutdown
================
*/
void App_Main::shutdown() {
    // make sure we exit event loop
    m.timer->stop();

    // shut down subsystems
    if ( !com.caughtFatalError ) {
        cmd.removeCommand( "exit" );
#ifndef YP2_FINAL_RELEASE
        cmd.removeCommand( "fatalError" );
#endif
        cv.saveConfig( DEFAULT_CONFIG_FILE );
        com.gui->shutdown();
        cmd.shutdown();
        fs.shutdown();
        cv.shutdown();
        mod.shutdown();
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
    QString command;

    // compile a list for easier handling
    for ( y = 1; y < argc; y++ )
        argList << argv[y];

    // nothing? - leave!
    if ( argList.isEmpty())
        return;

    // fast check
    if ( !argList.first().startsWith( "+" )) {
        com.error( ERR_SOFT, this->tr( "App_Main::parseArgs: arguments must start with '+'\n" ));
        return;
    }

    // reset counter
    y = 0;

    // compile commands and args, execute
    foreach ( QString str, argList ) {
        if (( y != 0 && str.startsWith( "+" ))) {
            cmd.execute( command.remove( "+" ));
            command.clear();
        }
        command.append( str + " " );
        y++;
    }
    cmd.execute( command.remove( "+" ));
}

/*
================
update
================
*/
void App_Main::update() {
    mod.update();
}

/*
================
cmdFatalError
================
*/
#ifndef YP2_FINAL_RELEASE
void App_Main::fatalError() {
    // for debugging (of gui in general)
    com.error( ERR_FATAL, this->tr( "cmdFatalError: we have failed\n" ));
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
    com.gui = &gui;

    // init subsystems
    cmd.init();
    cv.init();
    fs.init();
    mod.init();

    // parse config only AFTER filesystem has been initialized
    cv.parseConfig( DEFAULT_CONFIG_FILE );
    com.gui->init();

#ifndef YP2_FINAL_RELEASE
    cmd.addCommand( "fatalError", fatalErrorCmd, this->tr( "let's have some fish" ));
#endif
    cmd.addCommand( "exit", shutdownCmd, this->tr( "shutdown platform" ));

    // set defaults for event handling & update subSystems
    delay = ( 1000 / PLATFORM_UPDATE_FREQUENCY );
    this->timer = new QTimer( this );
    connect( this->timer, SIGNAL( timeout()), this, SLOT( update()));
    this->timer->start( delay );

    // print info
    com.gui->printImage( ":/icons/settings", 24, 24 );
    com.print( this->tr( " ^5Platform: ^3initialization complete\n" ), 12 );
    com.gui->printImage( ":/icons/about", 16, 16 );
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
