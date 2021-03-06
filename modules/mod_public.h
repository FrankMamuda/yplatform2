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

#ifndef MOD_PUBLIC_H
#define MOD_PUBLIC_H

namespace ModuleAPI {
    enum Flags {
        NoFlags             = 0x00,
        RendererLoaded      = 0x01
    };

    // console state (gui)
    enum ConsoleState {
        ConsoleUnlocked = 0,
        ConsoleLocked
    };
    // console state (gui)
    enum ToolBarActions {
        ActionSettings = 0,
        ActionModules,
        ActionAbout,
        ActionExit
    };
    // public API calls (modules)
    enum ModuleAPICalls {
        ModAPI = 0,
        Init,
        Update,
        UpdateCvar,
        Shutdown,
        KeyEvent,
        MouseEvent,
        MouseMotion,
        WheelEvent
    };
    // public API calls (platform)
    enum PlatformAPICalls {
        // commons
        ComPrint = 0,
        ComError,
        ComMilliseconds,

        // filesystem
        FsOpen,
        FsClose,
        FsCloseByName,
        FsExists,
        FsRead,
        FsWrite,
        FsSeek,
        FsTouch,
        FsReadFile,
        FsPrint,
        FsExtract,
        FsList,

        // command subsystem
        CmdAdd,
        CmdRemove,
        CmdExecute,
        CmdAddScripted,

        // cvars
        CvarCreate,
        CvarCreateInteger,
        CvarCreateValue,
        CvarCreateBoolean,
        CvarSet,
        CvarGet,
        CvarReset,

        // applet
        AppShutdown,

        // gui
        GuiRaise,
        GuiHide,
        GuiCreateSystray,
        GuiRemoveSystray,
        GuiRemoveAction,
        GuiAddToolBar,
        GuiRemoveToolBar,
        GuiRemoveMainToolBar,
        GuiAddTab,
        GuiRemoveTab,
        GuiSetActiveTab,
        GuiSetConsoleState,
        GuiAddSettingsTab,
        GuiRemoveSettingsTab,
        GuiShowTabWidget,
        GuiHideTabWidget,

        // platform flags
        PlatformFlags,

        // renderer only
        RendererKeyEvent = 100,
        RendererMouseEvent,
        RendererMouseMotion,
        RendererWheelEvent
    };
    enum KeyEventType {
        KeyPress = 0,
        KeyRelease,
        DoubleClick
    };
    enum Destination {
        Platform = 0,
        Renderer/*,
        Soundsystem*/
    };
    // api version (to avoid mismatch)
    const static unsigned int Version = 8;
}

#endif // MOD_PUBLIC_H
