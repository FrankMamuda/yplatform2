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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

//
// includes
//
#include "../common/sys_shared.h"
#include "../common/sys_cmd.h"
#include "../modules/mod_public.h"
#include "../gui/gui_settings.h"
#ifndef MODULE_BUILD
#include <QtGui>
#endif

//
// namespaces
//
namespace Ui {
    class Gui_Main;
    static const int MaxConsoleHistory = 32;
    static const int DefaultToolbarIconSize = 48;
    static const QString DefaultHistoryFile( "history.xml" );
    static const QString DefaultConsoleHTML( "platform/consoleText.html" );
}

typedef struct customTabDef_s {
    QString name;
    int index;
} customTabDef_t;

typedef struct imageResourceDef_s {
    QImage image;
    QString name;
} imageResourceDef_t;

//
// class:Gui_Main
//
class Gui_Main : public QMainWindow {
    Q_OBJECT
    Q_CLASSINFO( "description", "Platform main window" )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_PROPERTY( bool tray READ hasTray WRITE setTrayInitialized )
    Q_PROPERTY( bool visible READ isVisible WRITE setVisibility )
    Q_PROPERTY( bool newText READ hasNewText WRITE setNewText )
    Q_PROPERTY( bool historyChanged READ hasNewHistory WRITE setNewHistory )
    Q_PROPERTY( int  historyOffset READ historyOffset WRITE setHistoryOffset RESET resetHistoryOffset )
    Q_ENUMS( TabDestinations )

public:
    enum TabDestinations {
        MainWindow,
        Settings
    };

    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();
    QStringList cmdList;

    // property getters
    bool hasInitialized() const { return this->m_initialized; }
    bool hasTray() const { return this->m_tray; }
    bool isVisible() const { return this->m_visible; }
    bool hasNewText() const { return this->m_newText; }
    bool hasNewHistory() const { return this->m_historyChanged; }
    int  historyOffset() const { return this->m_historyOffset; }

protected:
    void changeEvent( QEvent * );
    void closeEvent( QCloseEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    Ui::Gui_Main *ui;
    QStringList lastMatch;
    QStringList history;
    QImage *addImageResource( const QString &filename, int w = 0, int h = 0 );
    bool completeCommand();

    // actions
    QAction *settigsAction;
    QAction *moduleAction;
    QAction *aboutAction;
    QAction *exitMainAction;

    // tabs
    QList <customTabDef_t*>tabWidgetTabs[2];

    // tray
    QMenu *trayIconMenu;
    QSystemTrayIcon *trayIcon;
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *exitAction;

    // resources
    QList <imageResourceDef_t*>imageResources;

    // properties
    bool m_initialized;
    bool m_tray;
    bool m_visible;
    bool m_newText;
    bool m_historyChanged;
    int  m_historyOffset;

    // settings
    Gui_Settings *settings;

    // previous height (for hiding base widget)
    int previousHeight;

public slots:
    // initialization & gui
    void init();
    void shutdown();
    void freeze();
    void hideOrMinimize();
    void setWindowFocus();
    void autoScroll();
    void addTabExt( TabDestinations dest, QWidget *widget, const QString &name, const QString &icon );
    void removeTabExt( TabDestinations dest, const QString &name );

    // systray
    void createSystemTray();
    void removeSystemTray();

    // toolbar & tab widget
    void removeAction( ModuleAPI::ToolBarActions );
    void setActiveTab( const QString &name );
    void hideTabWidget();
    void showTabWidget();
    void removeMainToolBar();

    // printing & console
    void print( const QString &msg, int fontSize = 10 );
    void printImage( const QString &filename, int w = 0, int h = 0 );
    void printHtml( const QString &html );
    void setConsoleState( ModuleAPI::ConsoleState state );
    void addToCompleter( const QString &cmd );
    void removeFromCompleter( const QString &cmd );
    void clearConsole();

    // property setters
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void setTrayInitialized( bool intialized = true ) { this->m_tray = intialized; }
    void setVisibility( bool visiblity = true ) { this->m_visible = visiblity; }
    void setNewText( bool text = true ) { this->m_newText = text; }
    void setNewHistory( bool hist = true ) { this->m_historyChanged = hist; }
    void setHistoryOffset( int offset ) { this->m_historyOffset = offset; }
    void resetHistoryOffset() { this->m_historyOffset = 0; }
    void pushHistoryOffset() { this->m_historyOffset++; }
    void popHistoryOffset() { this->m_historyOffset--; }

private slots:
    void actionAboutTriggered();
    void on_consoleInput_returnPressed();
    void createActions();
    void createTrayActions();
    void iconActivated( QSystemTrayIcon::ActivationReason reason );
    void toolBarIconSizeModified();

    // history
    void addToHistory( const QString &text );
    void saveHistory( const QString &filename );
    void loadHistory( const QString &filename );
};

#endif // GUI_MAIN_H
