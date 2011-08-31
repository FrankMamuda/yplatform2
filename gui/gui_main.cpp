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
#include <QtXml/QDomNode>
#include "gui_main.h"
#include "gui_about.h"
#include "ui_gui_main.h"
#include "../applet/app_main.h"
#include "../common/sys_common.h"
#include "../common/sys_cvar.h"
#include "../common/sys_cmd.h"
#include "../common/sys_filesystem.h"
#include "../common/sys_module.h"

//
// cvars
//
pCvar *gui_toolBarIconSize;

//
// commands
//
createCommandPtr( com.gui(), createSystemTray )
createCommandPtr( com.gui(), removeSystemTray )
createCommandPtr( com.gui(), hideOrMinimize )
createCommandPtr( com.gui(), setWindowFocus )
createCommandPtr( com.gui(), show )

/*
================
construct
================
*/
Gui_Main::Gui_Main( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::Gui_Main ) {
    // setup ui
    this->ui->setupUi( this );
    this->ui->consoleScreen->ensureCursorVisible();
    this->ui->consoleInput->installEventFilter( this );
    this->setFocus();
    this->setInitialized();
    this->setTrayInitialized( false );
}

/*
================
focus
================
*/
void Gui_Main::setWindowFocus() {
    this->activateWindow();
}

/*
================
hideOrMinimize
================
*/
void Gui_Main::hideOrMinimize() {
    if ( this->hasTray())
        this->hide();
    else
        this->showMinimized();
}

/*
================
init
================
*/
void Gui_Main::init() {
    // history stuff
    this->resetHistoryOffset();
    this->loadHistory( Ui::DefaultHistoryFile );

    // visuals
    gui_toolBarIconSize = cv.create( "gui_toolBarIconSize", QString( "%1" ).arg( Ui::DefaultToolbarIconSize ), pCvar::Archive );
    this->toolBarIconSizeModified();

    // dynamic check on toolBar icon size
    this->connect( gui_toolBarIconSize, SIGNAL( valueChanged( QString, QString )), this, SLOT( toolBarIconSizeModified()));

    // add systray cmd
    cmd.add( "gui_createSysTray", createSystemTrayCmd, this->tr( "create system tray icon" ));
    cmd.add( "gui_removeSysTray", removeSystemTrayCmd, this->tr( "remove system tray icon" ));
    cmd.add( "gui_raise", showCmd, this->tr( "raise main window" ));
    cmd.add( "gui_hide", hideOrMinimizeCmd, this->tr( "hide main window" ));
    cmd.add( "gui_setFocus", setWindowFocusCmd, this->tr( "set focus on main window" ));

    // add console icon
    this->ui->tabWidget->setTabIcon( 0, QIcon( ":/icons/console" ));

    // init settings
    this->settings = new Gui_Settings( this );

    // create actions
    this->createActions();
}

/*
================
toolBarIconSizeModified
================
*/
void Gui_Main::toolBarIconSizeModified() {
    // accept only valid sizes
    if ( gui_toolBarIconSize->integer() >= 8 && gui_toolBarIconSize->integer() <= 128 )
        this->ui->toolBar->setIconSize( QSize( gui_toolBarIconSize->integer(), gui_toolBarIconSize->integer()));
    else
        gui_toolBarIconSize->set( QString( "%1" ).arg( Ui::DefaultToolbarIconSize ));

}

/*
================
shutdown
================
*/
void Gui_Main::shutdown() {
    // save history
    this->saveHistory( Ui::DefaultHistoryFile );

    // remove cmds
    cmd.remove( "gui_createSysTray" );
    cmd.remove( "gui_removeSysTray" );
    cmd.remove( "gui_raise" );
    cmd.remove( "gui_hide" );
    cmd.remove( "gui_setFocus" );

    // remove actions
    foreach ( customActionDef_t *actionPtr, this->toolBarActions ) {
        this->ui->toolBar->removeAction( actionPtr->action );
        delete actionPtr->action;
        this->toolBarActions.removeOne( actionPtr );
        delete actionPtr;
    }
    this->toolBarActions.clear();

    // remove tabs
    this->tabWidgetTabs[Gui_Main::MainWindow].clear();
    this->tabWidgetTabs[Gui_Main::Settings].clear();

    // remove image resources
    foreach ( imageResourceDef_t *imagePtr, this->imageResources ) {
        this->imageResources.removeOne( imagePtr );
        delete imagePtr;
    }
    this->imageResources.clear();

    // remove tray if any
    this->removeSystemTray();

    // delete settings
    delete this->settings;

    // gui is down
    this->setInitialized( false );
}

/*
================
destruct
================
*/
Gui_Main::~Gui_Main() {
    this->history.clear();
    delete ui;
}

/*
===============
createActions
===============
*/
void Gui_Main::createActions() {
    // settings
    this->settigsAction = new QAction( QIcon( ":/icons/settings" ), this->tr( "Settings" ), this );
    this->connect( this->settigsAction, SIGNAL( triggered()), this->settings, SLOT( intializeCvars()));
    this->connect( this->settigsAction, SIGNAL( triggered()), this->settings, SLOT( exec()));
    this->ui->toolBar->addAction( this->settigsAction );
    this->toolBarActions << new customActionDef_t;
    this->toolBarActions.last()->action = this->settigsAction;

    // modules
    this->moduleAction = new QAction( QIcon( ":/icons/module" ), this->tr( "Modules" ), this );
    this->connect( this->moduleAction, SIGNAL( triggered()), &mod, SLOT( toggleWidget()));
    this->ui->toolBar->addAction( this->moduleAction );
    this->toolBarActions << new customActionDef_t;
    this->toolBarActions.last()->action = this->moduleAction;

    // about
    this->aboutAction = new QAction( QIcon( ":/icons/about" ), this->tr( "About" ), this );
    this->ui->toolBar->addAction( this->aboutAction );
    this->connect( this->aboutAction, SIGNAL( triggered()), this, SLOT( actionAboutTriggered()));
    this->toolBarActions << new customActionDef_t;
    this->toolBarActions.last()->action = this->aboutAction;

    // exit
    this->exitAction = new QAction( QIcon( ":/icons/exit" ), this->tr( "Exit" ), this );
    this->connect( this->exitAction, SIGNAL( triggered()), &m, SLOT( shutdown()));
    this->ui->toolBar->addAction( this->exitAction );
    this->toolBarActions << new customActionDef_t;
    this->toolBarActions.last()->action = this->exitAction;
}

/*
===============
createTrayActions
===============
*/
void Gui_Main::createTrayActions() {
    // minimize
    this->minimizeAction = new QAction( QIcon( ":/icons/minimize" ), this->tr( "Minimize" ), this );
    this->connect( this->minimizeAction, SIGNAL( triggered()), this, SLOT( hide()));

    // restore
    this->restoreAction = new QAction( QIcon( ":/icons/restore" ), this->tr( "Restore" ), this );
    this->connect( this->restoreAction, SIGNAL( triggered()), this, SLOT( showNormal()));
}

/*
================
createSystemTray
================
*/
void Gui_Main::createSystemTray() {
    // failsafe
    if ( this->hasTray())
        return;

    // check for sysTray
    if ( !QSystemTrayIcon::isSystemTrayAvailable()) {
        com.error( Sys_Common::SoftError, this->tr( "Gui_Main::createSystemTray: could not detect system tray\n" ));
        return;
    }

    // create actions first
    this->createTrayActions();

    // create tray menu
    this->trayIconMenu = new QMenu( this );
    this->trayIconMenu->addAction( this->minimizeAction );
    this->trayIconMenu->addAction( this->restoreAction );
    this->trayIconMenu->addSeparator();
    this->trayIconMenu->addAction( this->exitAction );

    this->trayIcon = new QSystemTrayIcon( QIcon( ":/icons/platform" ), this );
    this->trayIcon->setContextMenu( this->trayIconMenu );

    // connect
    this->connect( this->trayIcon, SIGNAL( activated( QSystemTrayIcon::ActivationReason )),
                   this, SLOT( iconActivated( QSystemTrayIcon::ActivationReason )));

    // display tray icon
    this->trayIcon->show();
    this->setTrayInitialized();
}

/*
===============
iconActivated
===============
*/
void Gui_Main::iconActivated( QSystemTrayIcon::ActivationReason reason ) {
    switch ( reason ) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:

        if ( !this->isVisible()) {
            this->setVisibility();
            this->show();
        } else {
            this->setVisibility( false );
            this->hide();
        }
        break;

    default:
        ;
    }
}

/*
================
removeSystemTray
================
*/
void Gui_Main::removeSystemTray() {
    // delete tray
    if ( this->hasTray()) {
        delete this->minimizeAction;
        delete this->restoreAction;
        delete this->trayIconMenu;
        delete this->trayIcon;
        this->setTrayInitialized( false );
    }
}

/*
================
changeEvent
================
*/
void Gui_Main::changeEvent( QEvent *e ) {
    QMainWindow::changeEvent(e);
    switch ( e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi( this );
        break;
    default:
        break;
    }
}

/*
================
eventFilter
================
*/
bool Gui_Main::eventFilter( QObject *object, QEvent *event ) {
    int sMatch = 0;
    int y;

    if ( object == this->ui->consoleInput ) {
        if ( this->ui->consoleInput->hasFocus()) {
            if ( event->type() == QEvent::KeyPress ) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );

                // history list -> up
                if ( keyEvent->key() == Qt::Key_Up ) {
                    if ( !this->history.isEmpty()) {
                        if ( this->historyOffset() < this->history.count() )
                            this->pushHistoryOffset();

                        int offset = this->history.count() - this->historyOffset();

                        if ( offset > 0 )
                            this->ui->consoleInput->setText( this->history.at( offset ));
                        else
                            this->ui->consoleInput->setText( this->history.first());
                    }
                    return true;

                    // history list -> down
                } else if ( keyEvent->key() == Qt::Key_Down ) {
                    if ( !this->history.isEmpty()) {
                        int offset;

                        if ( this->historyOffset() > 0 )
                            this->popHistoryOffset();

                        if ( this->historyOffset() == 0 ) {
                            this->ui->consoleInput->clear();
                            return true;
                        }

                        offset = this->history.count() - this->historyOffset();

                        if ( offset < this->history.count())
                            this->ui->consoleInput->setText( this->history.at( offset ));
                        else
                            this->ui->consoleInput->setText( this->history.last());
                    }
                    return true;

                    // complete command
                } else if ( keyEvent->key() == Qt::Key_Tab ) {
                    QStringList match;

                    // abort if no text at all
                    if ( this->ui->consoleInput->text().isEmpty())
                        return true;

                    foreach( QString str, this->cmdList ) {
                        if ( str.startsWith( this->ui->consoleInput->text(), Qt::CaseInsensitive ))
                            match << str;
                    }

                    // make sure we don't print same stuff all the time
                    if ( !this->hasNewText()) {
                        if ( match == this->lastMatch )
                            return true;
                    }

                    // complete to shortest string
                    if ( match.count() == 1 ) {
                        this->ui->consoleInput->setText( match.first() );
                    } else if ( match.count() > 1 ) {
                        sMatch = 1;
                        for ( y = 0; y < match.count(); y++ ) {
                            if ( match.first().at(sMatch) == match.at(y).at(sMatch)) {
                                if ( y == match.count()-1 ) {
                                    sMatch++;
                                    y = 0;
                                }
                            }
                        }
                        this->ui->consoleInput->setText( match.first().left( sMatch ));
                    } else if ( !match.count()) {
                        return true;
                    }

                    this->printImage( ":/icons/about", 16, 16 );
                    com.print( this->tr( " ^5Available commands and cvars:\n" ));
                    foreach ( QString str, match ) {
                        // check commands
                        pCmd *cmdPtr;
                        cmdPtr = cmd.find( str );
                        if ( cmdPtr != NULL ) {
                            if ( !cmdPtr->description().isEmpty()) {
                                com.print( QString( " ^3\"%1\"^5 - ^3%2\n" ).arg( str, cmdPtr->description()));
                            } else {
                                com.print( QString( " ^3\"%1\n" ).arg( str ));
                            }
                        }

                        // check variables
                        pCvar *cvarPtr = cv.find( str );

                        // perform a variable print or set
                        if ( cvarPtr != NULL ) {
                            if ( !cvarPtr->description().isEmpty()) {
                                com.print( this->tr( " ^3\"%1\" ^5is ^3\"%2\"^5 - ^3%3\n" ).arg(
                                               cvarPtr->name(),
                                               cvarPtr->string(),
                                               cvarPtr->description()
                                               ));
                            } else {
                                com.print( this->tr( " ^3\"%1\" ^5is ^3\"%2\"\n" ).arg(
                                               cvarPtr->name(),
                                               cvarPtr->string()));
                            }
                        }
                    }

                    com.print( "\n" );
                    this->lastMatch = match;
                    this->setNewText( false );
                    return true;
                }
            }
            return false;
        }
    }
    return QMainWindow::eventFilter( object, event );
}

/*
================
print
================
*/
void Gui_Main::print( const QString &message, int fontSize ) {
    int i;
    QString msg;

    // replace html specific symbols
    msg = message;
    msg.replace( "&", "&amp;" );
    msg.replace( " - ", "&nbsp;&mdash;&nbsp;" );
    msg.replace( " ", "&nbsp;" );
    msg.replace( ">", "&gt;" );
    msg.replace( "<", "&lt;" );
    msg.replace( "\n", "<br>" );

    // add default formatting
    msg.prepend( QString( "<span style=\"color:#FFFFFF; font-size:%1pt;\">" ).arg( fontSize ));

    // replace colours
    for ( i = 0; i < msg.length(); i++ ) {
        if ( msg.at( i ) == QChar( Sys::ColourEscape )) {
            // COLOUR_WHITE
            if ( msg.at( i + 1 ) == Sys::ColourBlack ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#FFFFFF; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_RED
            else if ( msg.at( i + 1 ) == Sys::ColourRed ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#FF0000; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_GREEN
            else if ( msg.at( i + 1 ) == Sys::ColourGreen ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#00FF00; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_YELLOW
            else if ( msg.at( i + 1 ) == Sys::ColourYellow ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#FFFF00; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_BLUE
            else if ( msg.at( i + 1 ) == Sys::ColourBlue ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#0000FF; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_CYAN
            else if ( msg.at( i + 1 ) == Sys::ColourCyan ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#00FFFF; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_MAGENTA
            else if ( msg.at( i + 1 ) == Sys::ColourMagenta ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#FF00FF; font-size:%1pt;\">" ).arg( fontSize ));
            }
            // COLOUR_BLACK
            else if ( msg.at( i + 1 ) == Sys::ColourWhite ) {
                msg.remove( i, 2 );
                msg.insert( i, QString( "</span><span style=\"color:#000000; font-size:%1pt;\">" ).arg( fontSize ));
            }
        }
    }
    msg.append( "</span>" );
    this->printHtml( msg );
}

/*
================
printHtml
================
*/
void Gui_Main::printHtml( const QString &html ) {
    // needs to be done before and after
    this->autoScroll();

    // print msg
    this->ui->consoleScreen->insertHtml( html );
    this->setNewText();
    this->autoScroll();
}

/*
================
autoScroll
================
*/
void Gui_Main::autoScroll() {
    QTextCursor cPos = this->ui->consoleScreen->textCursor();
    cPos.movePosition( QTextCursor::End );
    this->ui->consoleScreen->setTextCursor( cPos );
    this->ui->consoleScreen->ensureCursorVisible();
}

/*
================
addImageResource
================
*/
QImage *Gui_Main::addImageResource( const QString &filename, int width, int height ) {
    // failsafe
    foreach ( imageResourceDef_t *imgPtr, this->imageResources ) {
        if ( !QString::compare( filename, imgPtr->name )) {
            com.error( Sys_Common::SoftError, this->tr( "Gui_Main::addImageResource: image \"%1\" has already been added\n" ).arg( filename ));
            return NULL;
        }
    }

    // generate image
    byte *buffer;
    long len = fs.readFile( filename, &buffer, Sys_Filesystem::Silent );

    if ( len > 0 ) {
        this->imageResources << new imageResourceDef_t;
        if ( width > 0 && height > 0 )
            this->imageResources.last()->image = QImage::fromData( QByteArray (( const char* )buffer, len )).scaled( QSize( width, height ));
        else
            this->imageResources.last()->image = QImage::fromData( QByteArray (( const char* )buffer, len ));

        this->imageResources.last()->name = filename;
        this->ui->consoleScreen->document()->addResource( QTextDocument::ImageResource, QUrl( filename ), this->imageResources.last()->image );
        return &this->imageResources.last()->image;
    } else
        com.error( Sys_Common::SoftError, this->tr( "Gui_Main::addImageResource: could not add image \"%1\"\n" ).arg( filename ));

    return NULL;
}

/*
================
printImage
================
*/
void Gui_Main::printImage( const QString &filename, int width, int height ) {
    QImage *imagePtr = NULL;

    foreach ( imageResourceDef_t *imgPtr, this->imageResources ) {
        if ( !QString::compare( filename, imgPtr->name )) {
            imagePtr = &imgPtr->image;
            break;
        }
    }

    // if none, create one and retry
    if ( imagePtr == NULL ) {
        imagePtr = this->addImageResource( filename, width, height );

        if ( imagePtr == NULL )
            return;
    }

    // insert image
    this->autoScroll();
    QTextCursor cPos = this->ui->consoleScreen->textCursor();
    cPos.insertImage( *imagePtr );
    this->autoScroll();
}

/*
================
addToHistory
================
*/
void Gui_Main::addToHistory( const QString &text) {
    // ignore same stuff
    if ( this->history.count()) {
        if ( !QString::compare( this->history.last(), text ))
            return;
    }

    if ( this->history.count() >= Ui::MaxConsoleHistory )
        this->history.removeFirst();

    this->history << text;
    this->setNewHistory();
}

/*
================
consoleInput->returnPressed
================
*/
void Gui_Main::on_consoleInput_returnPressed() {
    if ( !this->ui->consoleInput->text().isEmpty()) {
        com.print( QString( "^2\%1\n" ).arg( this->ui->consoleInput->text()));

        if ( cmd.execute( this->ui->consoleInput->text()))
            this->addToHistory( this->ui->consoleInput->text());

        this->ui->consoleInput->clear();
        this->resetHistoryOffset();
    }
}

/*
================
addToCompleter
================
*/
void Gui_Main::addToCompleter( const QString &cmd ) {
    cmdList << cmd;
}

/*
================
removeFromCompleter
================
*/
void Gui_Main::removeFromCompleter( const QString &cmd ) {
    cmdList.removeAll( cmd );
}

/*
================
freeze
================
*/
void Gui_Main::freeze() {
    // this is called from FatalError
    // since we have already failed, there is no need to shut
    // down additional subsystems which can cause even more
    // errors and then we're stuck in a loop
    this->ui->consoleInput->setEnabled( false );

    if ( this->aboutAction != NULL )
        this->aboutAction->setEnabled( false );

    if ( this->settigsAction != NULL )
        this->settigsAction->setEnabled( false );

    this->ui->consoleScreen->setEnabled( false );
}

/*
================
actionAboutTriggered
================
*/
void Gui_Main::actionAboutTriggered() {
    Gui_About aDialog;

    // open up
    aDialog.exec();
}

/*
===============
saveHistory
===============
*/
void Gui_Main::saveHistory( const QString &filename ) {
    // create document
    QDomDocument histFile;

    if ( !this->hasNewHistory())
        return;

    // create history tag
    QDomElement histElement = histFile.createElement( "history" );
    histElement.setAttribute( "version", "1.0" );
    histElement.setAttribute( "generator", "YPlatform2" );
    histFile.appendChild( histElement );

    // generate cmd history strings
    foreach ( QString str, this->history ) {
        QDomElement cmdElement = histFile.createElement( "cmd" );
        histElement.appendChild( cmdElement );

        QDomText cmdText = histFile.createTextNode( str );
        cmdElement.appendChild( cmdText );
    }

    // write out
    fileHandle_t fileOut;
    if ( fs.open( pFile::Write, filename, fileOut, Sys_Filesystem::Silent ) != -1 ) {
        fs.print( fileOut, histFile.toString());
        fs.close( fileOut );
    }
}

/*
===============
loadHistory
===============
*/
void Gui_Main::loadHistory( const QString &filename ) {
    QDomDocument histFile;

    // read buffer
    byte *buffer;
    long len = fs.readFile( filename, &buffer, Sys_Filesystem::Silent );

    // failsafe
    if ( len == -1 )
        return;

    //
    // parse document
    //
    histFile.setContent( QByteArray(( const char* )buffer, len ));
    QDomNode histNode = histFile.firstChild();
    while ( !histNode.isNull()) {
        if ( histNode.isElement()) {
            QDomElement histElement = histNode.toElement();

            // check element name
            if ( QString::compare( histElement.tagName(), "history" )) {
                com.error( Sys_Common::SoftError, this->tr( "Gui_Main::loadHistory: expected <history> in \"%1\"\n" ).arg( filename ));
                return;
            }

            QDomNode cmdNode = histElement.firstChild();
            while ( !cmdNode.isNull()) {
                if ( cmdNode.isElement()) {
                    QDomElement cmdElement = cmdNode.toElement();

                    // check element name
                    if ( QString::compare( cmdElement.tagName(), "cmd" )) {
                        com.error( Sys_Common::SoftError, this->tr( "Gui_Main::loadHistory: expected <cmd> in \"%1\"\n" ).arg( filename ));
                        return;
                    }

                    this->history << cmdElement.text();
                }
                cmdNode = cmdNode.nextSibling();
            }
            break;
        }
        histNode = histNode.nextSibling();
    }

    // clear buffer
    fs.freeFile( filename );
}

/*
===============
closeEvent
===============
*/
void Gui_Main::closeEvent( QCloseEvent *event ) {
    m.shutdown();
    event->accept();
}

/*
===============
addToolBarAction

 dd: do we handle translated actions properly?
===============
*/
void Gui_Main::addToolBarAction( const QString &name, const QString &icon, cmdCommand_t callBack ) {
    // failsafe
    foreach ( customActionDef_t *actionPtr, this->toolBarActions ) {
        if ( !QString::compare( name, actionPtr->action->objectName())) {
            com.error( Sys_Common::SoftError, this->tr( "Gui_Main::addAction: action \"%1\" already exists\n" ).arg( name ));
            return;
        }
    }

    // create custom action
    this->toolBarActions << new customActionDef_t;
    this->toolBarActions.last()->action = new QAction( this );
    this->toolBarActions.last()->action->setObjectName( name );
    this->toolBarActions.last()->action->setText( name );

    // generate icon
    if ( !icon.isEmpty()) {
        if ( name.startsWith( ":" )) {
            this->toolBarActions.last()->action->setIcon( QIcon( name ));
        } else {
            byte *buffer;
            long len = fs.readFile( icon, &buffer, Sys_Filesystem::Silent );

            // any icon?
            if ( len > 0 ) {
                QPixmap pixMap;
                pixMap.loadFromData( QByteArray (( const char* )buffer, len ));
                this->toolBarActions.last()->action->setIcon( QIcon( pixMap ));
            }
        }
    }

    // add callback
    this->toolBarActions.last()->callBack = callBack;

    // connect
    connect( this->toolBarActions.last()->action, SIGNAL( triggered()), this, SLOT( customActionSlot()));

    // add to menuBar
    this->ui->toolBar->addAction( this->toolBarActions.last()->action );
}

/*
===============
removeAction
===============
*/
void Gui_Main::removeAction( const QString &name ) {
    foreach ( customActionDef_t *actionPtr, this->toolBarActions ) {
        if ( !QString::compare( name, actionPtr->action->objectName())) {
            this->ui->toolBar->removeAction( actionPtr->action );
            delete actionPtr->action;
            this->toolBarActions.removeOne( actionPtr );
        }
    }
}

/*
===============
customActionSlot
===============
*/
void Gui_Main::customActionSlot() {
    QAction* senderAction = qobject_cast<QAction *>( QObject::sender());
    foreach ( customActionDef_t *actionPtr, this->toolBarActions ) {
        if ( !QString::compare( senderAction->text(), actionPtr->action->text()))
            actionPtr->callBack();
    }
}

/*
===============
setConsoleState
===============
*/
void Gui_Main::setConsoleState( ModuleAPI::ConsoleState state ) {
    if ( state == ModuleAPI::ConsoleLocked )
        this->ui->tabConsole->setDisabled( true );
    else if ( ModuleAPI::ConsoleUnlocked )
        this->ui->tabConsole->setEnabled( true );
}

/*
===============
addTabExt
===============
*/
void Gui_Main::addTabExt( TabDestinations dest, QWidget *widget, const QString &name, const QString &icon ) {
    QTabWidget *destWidget;

    // check destination
    if ( dest == Gui_Main::MainWindow )
        destWidget = this->ui->tabWidget;
    else {
        destWidget = this->settings->settingsTabWidget;

        // external settings widgets must behave just like native widgets
        this->connect( this->settigsAction, SIGNAL( triggered()), widget, SLOT( intializeCvars()));
        this->connect( this->settings, SIGNAL( accepted()), widget, SLOT( saveCvars()));
    }

    // failsafe
    foreach ( customTabDef_t *customTabPtr, this->tabWidgetTabs[dest] ) {
        if ( !QString::compare( name, customTabPtr->name )) {
            com.error( Sys_Common::SoftError, this->tr( "Gui_Main::addTabExt: tab already exists\n" ));
            return;
        }
    }

    // generate icon
    if ( !icon.isEmpty()) {
        if ( icon.startsWith( ":" )) {
            destWidget->addTab( widget, QIcon( icon ), name );
        } else {
            byte *buffer;
            long len = fs.readFile( icon, &buffer, Sys_Filesystem::Silent );

            // any icon?
            if ( len > 0 ) {
                QPixmap pixMap;
                pixMap.loadFromData( QByteArray (( const char* )buffer, len ));
                destWidget->addTab( widget, QIcon( pixMap ), name );
            } else
                destWidget->addTab( widget, name );
        }
    } else
        destWidget->addTab( widget, name );

    // dd, store index
    this->tabWidgetTabs[dest] << new customTabDef_t;
    this->tabWidgetTabs[dest].last()->name = name;
    this->tabWidgetTabs[dest].last()->index = destWidget->count() - 1;
}

/*
===============
removeTabExt
===============
*/
void Gui_Main::removeTabExt( TabDestinations dest, const QString &name ) {
    QTabWidget *destWidget;
    bool reIndex;
    reIndex = false;

    // check destination
    if ( dest == Gui_Main::MainWindow )
        destWidget = this->ui->tabWidget;
    else
        destWidget = this->settings->settingsTabWidget;

    foreach ( customTabDef_t *customTabPtr, this->tabWidgetTabs[dest] ) {
        if ( !QString::compare( name, customTabPtr->name )) {
            reIndex = true;
            destWidget->removeTab( customTabPtr->index );
        }

        if ( reIndex )
            customTabPtr->index--;
    }
}

/*
===============
setActiveTab
===============
*/
void Gui_Main::setActiveTab( const QString &name ) {
    foreach ( customTabDef_t *customTabPtr, this->tabWidgetTabs[Gui_Main::MainWindow] ) {
        if ( !QString::compare( name, customTabPtr->name )) {
            this->ui->tabWidget->setCurrentIndex( customTabPtr->index );
        }
    }
}
