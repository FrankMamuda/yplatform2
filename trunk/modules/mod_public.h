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

#ifndef MOD_PUBLIC_H
#define MOD_PUBLIC_H

// console state (gui)
#define CONSOLE_STATE_LOCKED    0
#define CONSOLE_STATE_UNLOCKED  1

//
// public API calls
//
enum {
    MOD_API = 0,
    MOD_INIT,
    MOD_UPDATE,
    MOD_UPDATE_CVAR,
    MOD_SHUTDOWN
};

enum {
    // commons
    COM_PRINT = 0,
    COM_ERROR,
    COM_MILLISECONDS,

    // filesystem
    FS_FOPEN_FILE,
    FS_FCLOSE_FILE,
    FS_FCLOSE_FILE_BY_NAME,
    FS_FILE_EXISTS,
    FS_READ,
    FS_WRITE,
    FS_SEEK,
    FS_TOUCH,
    FS_READ_FILE,
    FS_FPRINT,
    FS_FREE_FILE,
    FS_EXTRACT,
    FS_LIST_FILES,

    // command subsystem
    CMD_ADD_COMMAND,
    CMD_REMOVE_COMMAND,
    CMD_ARGC,
    CMD_ARGV,
    CMD_EXECUTE,

    // cvars
    CVAR_CREATE,
    CVAR_SET,
    CVAR_RESET,

    // applet
    APP_SHUTDOWN,

    // gui
    GUI_RAISE,
    GUI_HIDE,
    GUI_CREATE_SYSTRAY,
    GUI_REMOVE_SYSTRAY,
    GUI_ADD_ACTION,
    GUI_REMOVE_ACTION,
    GUI_ADD_TAB,
    GUI_REMOVE_TAB,
    GUI_SET_ACTIVE_TAB,
    GUI_SET_CONSOLE_STATE
};

//
// API version (to avoid mismatch)
//
#define MODULE_API_VERSION 1

#endif // MOD_PUBLIC_H
