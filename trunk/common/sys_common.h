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

#ifndef SYS_COMMON_H
#define SYS_COMMON_H

//
// includes
//
#include "sys_shared.h"
#include "../gui/gui_main.h"

// error types
enum {
    ERR_SOFT,
    ERR_FATAL
};

//
// class::Sys_Common
//
class Sys_Common : public QObject {
    Q_OBJECT

public:
    void print( const QString &msg, int fontSize = 10 );
    Gui_Main *gui;
    void error( int type, const QString &msg );
    bool caughtFatalError;
    int milliseconds();
    void strncpyz( char *, const char *, int );

private:
    unsigned long timeBase;

signals:

public slots:

};

// byte order
short shortSwap( short );
int longSwap( int );
float floatSwap( const float* );

typedef union {
    float   f;
    unsigned int i;
} floatByteUnion;

#endif // SYS_COMMON_H
