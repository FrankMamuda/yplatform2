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

#ifndef APPMAIN_H
#define APPMAIN_H

//
// includes
//
#include "../common/sys_shared.h"

//
// namespaces
//
namespace Main {
    class App_Main;
    static const unsigned int PlatformUpdateFrequency = 50;
}

//
// class:App_Main
//
class App_Main : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Application platform" )

public:
    int startup( int, char*[] );


public slots:
    void update();

    // commands
    void shutdown();
#ifndef YP2_FINAL_RELEASE
    void fatalError();
#endif

private:
    void parseArgs( int, char*[] );
    QTimer *timer;
};

//
// externals
//
#ifndef MODULE_LIBRARY
extern class App_Main m;
#endif

#endif
