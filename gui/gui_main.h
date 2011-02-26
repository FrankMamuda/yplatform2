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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

//
// includes
//
#include "../common/sys_shared.h"
#include "../common/sys_cmd.h"
#include "../modules/mod_public.h"
#ifndef MODULE_BUILD
#include <QtGui>
#endif

//
// defines
//
#define MAX_CONSOLE_HISTORY 32
#define DEFAULT_HISTORY_FILE "history.xml"
#define DEFAULT_TOOBAR_ICON_SIZE 48

//
// namespaces
//
namespace Ui {
class Gui_Main;
}

// custom actions
typedef struct customActionDef_s {
    QAction *action;
    cmdCommand_t callBack;
} customActionDef_t;

typedef struct customTabDef_s {
    QString name;
    int index;
} customTabDef_t;

typedef struct imageResourceDef_s {
    QImage image;
    QString name;
} imageResourceDef_t;


//
// classes
//
class Gui_Main : public QMainWindow {
    Q_OBJECT

public:
    explicit Gui_Main( QWidget *parent = 0 );
    ~Gui_Main();
    void print( const QString &msg, int fontSize = 10 );
    void printHtml( const QString &html );
    void addToCompleter( const QString &cmd );
    void removeFromCompleter( const QString &cmd );
    QStringList cmdList;
    bool newConsoleText;
    void saveHistory( const QString &filename );
    void loadHistory( const QString &filename );
    bool initialized;
    bool trayInitialized;
    bool visible;
    void addToolBarAction( const QString &name, const QString &icon, cmdCommand_t callBack );
    void removeAction( const QString &name );
    void addTab( QWidget *widget, const QString &name, const QString &icon );
    void removeTab( const QString &name );
    void setActiveTab( const QString &name );
    void setConsoleState( int state );
    void printImage( const QString &filename, int w = 0, int h = 0 );

protected:
    void changeEvent( QEvent *e );
    void closeEvent( QCloseEvent *event );
    bool eventFilter( QObject *object, QEvent *event );

private:
    Ui::Gui_Main *ui;
    void addToHistory( const QString &text );
    QStringList lastMatch;
    QStringList history;
    int historyOffset;
    bool historyChanged;
    QImage *addImageResource( const QString &filename, int w = 0, int h = 0 );

    // actions
    QAction *aboutAction;
    QAction *settigsAction;
    QAction *exitMainAction;
    QList <customActionDef_t*>toolBarActions;

    // tabs
    QList <customTabDef_t*>tabWidgetTabs;

    // tray
    QMenu *trayIconMenu;
    QSystemTrayIcon *trayIcon;
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *exitAction;
    QAction *moduleAction;

    // resources
    QList <imageResourceDef_t*>imageResources;

public slots:
    void shutdown();
    void init();
    void freeze();
    void removeSystemTray();
    void createSystemTray();
    void hideOrMinimize();

private slots:
    void actionAboutTriggered();
    void on_consoleInput_returnPressed();
    void createActions();
    void createTrayActions();
    void iconActivated( QSystemTrayIcon::ActivationReason reason );
    void customActionSlot();
    void toolBarIconSizeModified();
};

#endif // GUI_MAIN_H
