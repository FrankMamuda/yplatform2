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
#include "sys_common.h"
#include "sys_module.h"
#include "sys_filesystem.h"
#include "sys_cvar.h"
#include "sys_cmd.h"
#include "applet/app_main.h"

//
// classes
//
extern class Sys_Common com;
extern class Sys_Cmd cmd;
extern class Sys_Filesystem fs;
extern class Sys_Cvar cv;
extern class App_Main m;
class Sys_Module mod;

//
// cvars
//
pCvar *mod_extract;

//
// commands
//
createCommand( mod, load )
createCommand( mod, unload )

/*
============
parseManifest
============
*/
pModule *Sys_Module::parseManifest( const QString &filename ) {
    QDomDocument manifest;
    pModule *modPtr = NULL;
    int len;
    bool foundModule = false;

    // just allocate a new pointer with the current filename
    modPtr = new pModule( filename );

    // read buffer
    byte *buffer;
    len = fs.readFile( filename, &buffer, FS_FLAGS_SILENT );

    // failsafe
    if ( len <= 0 ) {
        modPtr->errorMessage = this->tr( "Invalid manifest" );
        return modPtr;
    }

    //
    // parse document
    //
    manifest.setContent( QByteArray(( const char* )buffer, len ));
    QDomNode moduleNode = manifest.firstChild();
    while ( !moduleNode.isNull()) {
        if ( moduleNode.isElement()) {
            QDomElement moduleElement = moduleNode.toElement();

            // check element name
            if ( QString::compare( moduleElement.tagName(), "module" )) {
                modPtr->errorMessage = this->tr( "Manifest syntax error: expected <module>" );
                return modPtr;
            } else
                foundModule = true;

            // entries must have names
            if ( moduleElement.hasAttribute( "name" )) {
                QString modName = moduleElement.attribute( "name" );

                // set the parsed name
                modPtr->name = modName;
            } else {
                modPtr->errorMessage = this->tr( "Module missing name" );
                return modPtr;
            }

            QDomNode infoNode = moduleElement.firstChild();
            while ( !infoNode.isNull()) {
                if ( infoNode.isElement()) {
                    QDomElement infoElement = infoNode.toElement();

                    // check element name
                    if ( !QString::compare( infoElement.tagName(), "description", Qt::CaseInsensitive ))
                        modPtr->description = infoElement.text();
                    else if ( !QString::compare( infoElement.tagName(), "icon", Qt::CaseInsensitive ))
                        modPtr->icon = infoElement.text();
                    else if ( !QString::compare( infoElement.tagName(), "api", Qt::CaseInsensitive )) {
                        modPtr->apiVersion = infoElement.text().toInt();
                        if ( modPtr->apiVersion > MODULE_API_VERSION ) {
                            modPtr->errorMessage = this->tr( "API version mismatch - %1, expected less or equal to %2" ).arg( modPtr->apiVersion ).arg( MODULE_API_VERSION );
                            return modPtr;
                        }
                    } else if ( !QString::compare( infoElement.tagName(), "filename", Qt::CaseInsensitive ))
                        modPtr->filename = infoElement.text();
                    else if ( !QString::compare( infoElement.tagName(), "version", Qt::CaseInsensitive ))
                        modPtr->versionString = infoElement.text();
                    else {
                        modPtr->errorMessage = this->tr( "Manifest syntax error: expected module info element, found <%1>" ).arg( infoElement.tagName());
                        return modPtr;
                    }
                }
                infoNode = infoNode.nextSibling();
            }
            break;
        }
        moduleNode = moduleNode.nextSibling();
    }

    // is the manifest empty?
    if ( !foundModule )
        modPtr->errorMessage = this->tr( "Missing module entry in manifest" );

    // clear buffer
    fs.freeFile( filename );

    // return the new module
    return modPtr;
}

/*
============
load
============
*/
void Sys_Module::load() {
    if ( cmd.argc() < 2 ) {
        com.print( this->tr( "^3usage: mod_load [module name]\n" ));
        return;
    }

    // find module
    QString mName = cmd.argv( 1 );

    // loading actual modules, not manifest filenames
    QList<QListWidgetItem*>list = this->modListWidget->findItems( mName, Qt::MatchExactly );
    foreach ( QListWidgetItem *item, list ) {
        foreach ( pModule *modPtr, this->preCachedList ) {
            if ( !QString::compare( mName, modPtr->name )) {
                if ( !modPtr->loaded )
                    this->toggleFromList( item );
                else
                    com.print( this->tr( "^3Sys_Module::load: module \"%1\" has already been loaded\n" ).arg( modPtr->name ));
            }
        }
    }
}

/*
============
unload
============
*/
void Sys_Module::unload() {
    if ( cmd.argc() < 2 ) {
        com.print( this->tr( "^3usage: mod_unload [module name]\n" ));
        return;
    }

    // find module
    QString mName = cmd.argv( 1 );

    // unloading actual modules, not manifest filenames
    QList<QListWidgetItem*>list = this->modListWidget->findItems( mName, Qt::MatchExactly );
    foreach ( QListWidgetItem *item, list ) {
        foreach ( pModule *modPtr, this->preCachedList ) {
            if ( !QString::compare( mName, modPtr->name )) {
                if ( modPtr->loaded )
                    this->toggleFromList( item );
                else
                    com.print( this->tr( "^3Sys_Module::load: module \"%1\" has already been unloaded\n" ).arg( modPtr->name ));
            }
        }
    }
}

/*
============
update
============
*/
void Sys_Module::update() {
    foreach ( pModule *modPtr, this->modList )
        modPtr->update();
}

/*
============
init
============
*/
void Sys_Module::init() {
    // add commands
    cmd.addCommand( "mod_load", loadCmd, this->tr( "load module by name" ));
    cmd.addCommand( "mod_unload", unloadCmd, this->tr( "unload module" ));

    // init cvars
    mod_extract = cv.create( "mod_extract", "1", CVAR_ARCHIVE, "toggle module copying from packages" );

    // precache modules
    this->preCacheModules();

    // create widget
    this->createWidget();
}

/*
============
shutdown
============
*/
void Sys_Module::shutdown() {
    // remove commands
    cmd.removeCommand( "mod_load" );
    cmd.removeCommand( "mod_unload" );

    // destroy widget
    this->destroyWidget();

    // unload all modules
    foreach( pModule *modPtr, this->modList )
        modPtr->unload();
    this->modList.clear();

    // delete all precached modules
    foreach( pModule *modPtr, this->preCachedList )
        delete modPtr;
    this->modList.clear();
}

/*
=================
platformSyscall
=================
*/
intptr_t Sys_Module::platformSyscalls( int callNum, int numArgs, intptr_t *args ) {
    switch ( callNum ) {

    //
    // commons
    //
    case COM_PRINT:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: COM_PRINT [message]\n" ));
            return false;
        }

        com.print(( const char * )args[0] );
        break;

    case COM_ERROR:
        if ( numArgs < 2 )  {
            com.error( ERR_SOFT, this->tr( "platformSyscall: COM_ERROR [type] [message]\n" ));
            return false;
        }

        com.error( args[0], ( const char * )args[1] );
        break;

    case COM_MILLISECONDS:
        return com.milliseconds();

        //
        // filesystem
        //
    case FS_FOPEN_FILE:
    {
        if ( numArgs < 4 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_FOPENFILE [mode] [filename] (&handle) (flags)\n" ));
            return false;
        }

        return fs.fOpenFile(( int )args[0], ( const char * )args[1], *( fileHandle_t *)args[2], ( int )args[3] );
    }

    case FS_FCLOSE_FILE:
        if ( numArgs < 2 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_FCLOSE_FILE [handle] (flags)\n" ));
            return false;
        }

        fs.fCloseFile(( fileHandle_t )args[0], ( int )args[1] );
        break;

    case FS_FCLOSE_FILE_BY_NAME:
        if ( numArgs < 2 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_FCLOSE_FILE [filename] (flags)\n" ));
            return false;
        }

        fs.fCloseFile(( const char * )args[0], ( int )args[1] );
        break;

    case FS_FILE_EXISTS:
        if ( numArgs < 3 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_FILEEXISTS [path] (flags)\n" ));
            return false;
        }
        return fs.fileExists(( const char * )args[0], ( int )args[1] );

    case FS_READ:
        if ( numArgs < 4 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_READ [&buffer] [len] [handle] (flags)\n" ));
            return false;
        }

        return fs.read(*( byte** )args[0], ( int )args[1], ( const fileHandle_t )args[2], ( int )args[3] );

    case FS_WRITE:
        // we cannot allow inappropriate calls to fs write! -> fatal error
        if ( numArgs < 4 ) {
            com.error( ERR_FATAL, this->tr( "platformSyscall: FS_WRITE [&buffer] [len] [handle] (flags)\n" ));
            return false;
        }

        return fs.write(( const byte* )args[0], ( int )args[1], ( const fileHandle_t )args[2], ( int )args[3] );

    case FS_SEEK:
        if ( numArgs < 4 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_SEEK [handle] [offset] (flags) (seek mode)\n" ));
            return false;
        }

        return fs.seek(( fileHandle_t )args[0], ( long )args[1], ( int )args[2], ( int )args[3] );

    case FS_TOUCH:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_TOUCH [filename] (flags)\n" ));
            return false;
        }

        fs.touch(( const char * )args[0], ( int )args[1] );
        break;

    case FS_READ_FILE:
        if ( numArgs < 3 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_READ_FILE [filename] [&buffer] (flags)\n" ));
            return false;
        }

        return fs.readFile(( const char * )args[0], ( byte** )args[1], ( int )args[2] );
        break;

    case FS_FPRINT:
        if ( numArgs < 3 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_FPRINT [handle] [msg] (flags)\n" ));
            return false;
        }

        fs.fPrint(( fileHandle_t )args[0], ( const char * )args[1], ( int )args[2] );
        break;

    case FS_FREE_FILE:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_FREE_FILE [filename]\n" ));
            return false;
        }

        fs.freeFile(( const char * )args[0] );
        break;

    case FS_EXTRACT:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_EXTRACT [filename]\n" ));
            return false;
        }

        return fs.extractFromPackage( ( const char * )args[0] );

    case FS_LIST_FILES:
    {
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: FS_LIST_FILES [directory] (filter) (mode)\n" ));
            return false;
        }
        // beware: don't use semicolon in dirname... but why should you in the first place?
        return ( intptr_t )fs.list(( const char * )args[0], *( const QRegExp* )( const char *)args[1], ( int )args[2] ).join( ";" ).toLatin1().constData();
    }
        break;

        //
        // command subsystem
        //
    case CMD_ADD_COMMAND:
        if ( numArgs < 3 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CMD_ADD_COMMAND [cmdName] [cmd] (description)\n" ));
            return false;
        }

        cmd.addCommand(( const char * )args[0], ( cmdCommand_t )args[1], ( const char * )args[2] );
        break;

    case CMD_REMOVE_COMMAND:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CMD_REMOVE_COMMAND [cmdName]\n" ));
            return false;
        }

        cmd.removeCommand(( const char * )args[0] );
        break;

    case CMD_ARGC:
        return cmd.argc();

    case CMD_ARGV:
    {
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CMD_ARGV [argId] [&buffer]\n" ));
            return false;
        }
        return ( intptr_t )cmd.argv( 1 ).toLatin1().constData();
    }
        break;

    case CMD_EXECUTE:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CMD_EXECUTE [command string]\n" ));
            return false;
        }

        return cmd.execute(( const char * )args[0] );

        //
        // cvar subsystem
        //
    case CVAR_CREATE:
        if ( numArgs < 4 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CVAR_CREATE [name] [string] (flags) (description)\n" ));
            return false;
        }
        if ( cv.create(( const char * )args[0], ( const char * )args[1], ( int )args[2], ( const char * )args[3], true ) != NULL )
            return true;
        else
            return false;

    case CVAR_SET:
    {
        if ( numArgs < 3 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CVAR_SET [name] [value] (force)\n" ));
            return false;
        }
        pCvar *cvarPtr = cv.find(( const char * )args[0] );
        if ( cvarPtr != NULL ) {
            cvarPtr->set(( const char * )args[1], ( bool )args[2] );
            return true;
        } else
            return false;

    }
        break;

    case CVAR_RESET:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: CVAR_RESET [name]\n" ));
            return false;
        }

        cmd.execute( QString( "cv_reset %1" ).arg(( const char * )args[0] ));
        break;

        //
        // applet
        //
    case APP_SHUTDOWN:
        // shut down, no questions asked
        m.shutdown();
        break;

        //
        // gui
        //
    case GUI_RAISE:
        com.gui->show();
        break;

    case GUI_HIDE:
        com.gui->hide();
        break;

    case GUI_CREATE_SYSTRAY:
        com.gui->createSystemTray();
        break;

    case GUI_REMOVE_SYSTRAY:
        com.gui->removeSystemTray();
        break;

    case GUI_ADD_ACTION:
        if ( numArgs < 3 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: GUI_ADD_ACTION [name] [icon] [callback]\n" ));
            return false;
        }

        com.gui->addToolBarAction(( const char * )args[0], ( const char * )args[1], ( cmdCommand_t )args[3] );
        break;

    case GUI_REMOVE_ACTION:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: GUI_REMOVE_ACTION [name]\n" ));
            return false;
        }
        com.gui->removeAction(( const char * )args[0] );
        break;

    case GUI_ADD_TAB:
        if ( numArgs < 2 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: GUI_ADD_TAB [widgetPtr] [name] (icon)\n" ));
            return false;
        }
        com.gui->addTab(( QWidget* )args[0], ( const char * )args[1], ( const char * )args[2] );
        break;

    case GUI_REMOVE_TAB:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: GUI_REMOVE_TAB [name]\n" ));
            return false;
        }
        com.gui->removeTab(( const char * )args[0] );
        break;

    case GUI_SET_ACTIVE_TAB:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: GUI_SET_ACTIVE_TAB [name]\n" ));
            return false;
        }
        com.gui->setActiveTab(( const char * )args[0] );
        break;

    case GUI_SET_CONSOLE_STATE:
        if ( numArgs < 1 ) {
            com.error( ERR_SOFT, this->tr( "platformSyscall: GUI_SET_CONSOLE_STATE [state]\n" ));
            return false;
        }
        com.gui->setConsoleState(( int )args[0] );
        break;

    default:
        // we cannot accept calls from a fault module - might damage filesystem or else
        // abort!
        com.error( ERR_FATAL, this->tr( "platformSyscall: unknown callNum %1\n" ).arg( callNum ));
        return false;
    }
    return true;
}

/*
============
updateCvar
============
*/
void Sys_Module::updateCvar( const QString &cvar, const QString &stringValue ) {
    foreach ( pModule *modPtr, this->modList )
        modPtr->call( MOD_UPDATE_CVAR, ( intptr_t )cvar.toLatin1().constData(), ( intptr_t )stringValue.toLatin1().constData());
}

/*
============
moduleSort (sorting: valid first, a-z descending)
============
*/
bool moduleSort( const pModule *m1, const pModule *m2 ) {
    if ( m1->errorMessage.isEmpty() && m2->errorMessage.isEmpty())
        return m1->name.toLower() < m2->name.toLower();
    else if ( m1->errorMessage.isEmpty() && !m2->errorMessage.isEmpty())
        return true;
    else if ( !m1->errorMessage.isEmpty() && m2->errorMessage.isEmpty())
        return false;
    else if ( !m1->errorMessage.isEmpty() && !m2->errorMessage.isEmpty())
        return m1->name.toLower() < m2->name.toLower();

    return false;
}

/*
============
reCacheModules
============
*/
void Sys_Module::reCacheModules() {
    this->preCacheModules();

    // announce
    com.print( this->tr( "^2Sys_Module::reCacheModules: ^5recaching modules\n" ));

    // rather dirrty, but a working solution
    // removing those manifests already loaded
    foreach ( pModule *modPtr, this->preCachedList ) {
        foreach ( pModule *modPtrLoaded, this->modList ) {
            if ( !QString::compare( modPtr->name, modPtrLoaded->name ))
                this->preCachedList.removeOne( modPtr );
        }
    }

    // repopulate list with loaded modules
    foreach ( pModule *modPtr, this->modList )
        this->preCachedList << modPtr;

    // sort the list
    qSort( this->preCachedList.begin(), this->preCachedList.end(), moduleSort );

    // repopulate list in gui
    this->populateListWidget();
}

/*
============
preCacheModules
============
*/
void Sys_Module::preCacheModules() {
    QRegExp mFilter( "*.xml" );
    mFilter.setPatternSyntax( QRegExp::Wildcard );
    QStringList mList = fs.list( "modules/", mFilter, FS_LIST_FILES_ONLY );
    this->preCachedList.clear();

    // find and attempt to parse all manifests
    foreach ( QString mStr, mList )
        this->preCachedList << this->parseManifest( mStr );

    // sort the list
    qSort( this->preCachedList.begin(), this->preCachedList.end(), moduleSort );
}

/*
============
populateListWidget
============
*/
void Sys_Module::populateListWidget() {
    QList<QListWidgetItem*>list;
    foreach ( QListWidgetItem *item, list )
        delete item;
    this->modListWidget->clear();

    // find icon and add to list
    foreach ( pModule *modPtr, this->preCachedList ) {
        if ( modPtr->errorMessage.isEmpty()) {
            QIcon tempIcon;

            if ( !modPtr->icon.isEmpty()) {
                if ( modPtr->icon.startsWith( ":" )) {
                    tempIcon = QIcon( modPtr->icon );
                } else {
                    byte *buffer;
                    int len = fs.readFile( modPtr->icon, &buffer, FS_FLAGS_SILENT );

                    // any icon?
                    if ( len > 0 ) {
                        QPixmap pixMap;
                        pixMap.loadFromData( QByteArray (( const char* )buffer, len ));
                        tempIcon = QIcon( pixMap );
                    }
                }
            }

            if ( tempIcon.isNull())
                tempIcon = QIcon( ":/icons/module" );

            QListWidgetItem *moduleItem = new QListWidgetItem( tempIcon, modPtr->name );
             if ( modPtr->loaded ) {
                 // change colour & font in list
                 QFont font = moduleItem->font();
                 font.setBold( true );
                 moduleItem->setFont( font );
                 moduleItem->setBackground( QBrush( QColor( 0, 128, 0, 64 )));
             }

            this->modListWidget->addItem( moduleItem );
        } else
            this->modListWidget->addItem( new QListWidgetItem( QIcon( ":/icons/panic" ), modPtr->name ));
    }
}

/*
============
createWidget
============
*/
void Sys_Module::createWidget() {
    // create list widget
    this->modListWidget = new QListWidget( com.gui );
    this->modListWidget->setIconSize( QSize( 32, 32 ));
    this->modListWidget->setAlternatingRowColors( true );

    // add entries
    this->populateListWidget();

    // connect
    this->connect( this->modListWidget, SIGNAL( itemDoubleClicked( QListWidgetItem* )), this, SLOT( listWidgetAction()));

    // simple widget
    this->modWidget = new QWidget();
    this->modWidget->setGeometry( 0, 0, 400, 300 );
    this->modWidget->setWindowTitle( "Modules" );
    this->modWidget->setWindowIcon( QIcon( ":/icons/module" ));

    // create layout
    this->mLayout = new QGridLayout();
    this->modWidget->setLayout( this->mLayout );

    // add module list
    this->mLayout->addWidget( this->modListWidget );

    // create button layout
    this->bLayout = new QBoxLayout( QBoxLayout::LeftToRight );

    //
    // create & connect buttons
    //
    // load
    this->loadButton = new QPushButton( QIcon( ":/icons/exec" ), this->tr( "Load/Unload" ));
    this->connect( this->loadButton, SIGNAL( clicked()), this, SLOT( listWidgetAction()));
    this->bLayout->addWidget( this->loadButton );

    // recache modules
    this->refreshButton = new QPushButton( QIcon( ":/icons/refresh" ),this->tr( "Refresh" ));
    this->connect( this->refreshButton, SIGNAL( clicked()), this, SLOT( reCacheModules()));
    this->bLayout->addWidget( this->refreshButton );

    // close
    this->closeButton = new QPushButton( QIcon( ":/icons/close" ), this->tr( "Close" ));
    this->closeButton->setAutoDefault( true );
    this->closeButton->setDefault( true );
    this->connect( this->closeButton, SIGNAL( clicked()), this->modWidget, SLOT(hide()));
    this->bLayout->addWidget( this->closeButton );

    // add buttonLayout to widget
    this->mLayout->addLayout( bLayout, 1, 0 );
}

/*
============
destroyWidget
============
*/
void Sys_Module::destroyWidget() {
    // delete every possible thing
    delete this->loadButton;
    delete this->refreshButton;
    delete this->closeButton;
    delete this->bLayout;
    delete this->mLayout;

    // clear all items
    QList<QListWidgetItem*>list;
    foreach ( QListWidgetItem *item, list )
        delete item;
    this->modListWidget->clear();

    // get rid of the widget
    delete this->modListWidget;
}

/*
============
toggleWidget
============
*/
void Sys_Module::toggleWidget() {
    if ( this->modWidget != NULL ) {
        if ( this->modWidget->isVisible())
            this->modWidget->hide();
        else {
            // position it properly
            this->modWidget->move( QPoint( com.gui->pos().x() + com.gui->width()/2 - this->modWidget->width()/2, com.gui->pos().y()));
            this->modWidget->showNormal();
        }
    }
}

/*
============
itemLoadError
============
*/
void Sys_Module::itemLoadError( const QString &errorMessage ) {
    QMessageBox msgBox( this->modListWidget );
    msgBox.setIconPixmap( QPixmap( ":/icons/panic" ).scaled( 48, 48 ));
    msgBox.setWindowTitle( "Error" );
    msgBox.setWindowIcon( QIcon( ":/icons/panic" ));
    msgBox.setText( errorMessage );
    msgBox.exec();
}

/*
============
toggleFromList
============
*/
void Sys_Module::toggleFromList( QListWidgetItem *item ) {
    // deselect, so we see the coloured background
    this->modListWidget->clearSelection();

    // find it
    foreach ( pModule *modPtr, this->preCachedList ) {
        if ( !QString::compare( modPtr->name, item->text())) {
            // is it errorous already?
            if ( modPtr->errorMessage.isEmpty()) {
                // unload if loaded
                if ( modPtr->loaded ) {
                    // perform unloading, mark as unloaded
                    modPtr->unload();
                    modPtr->loaded = false;
                    this->modList.removeOne( modPtr );

                    // change colour & font in list
                    QFont font = item->font();
                    font.setBold( false );
                    item->setFont( font );
                    item->setBackground( QBrush( QColor( 0, 0, 0, 0 )));
                    break;
                } else {
                    // perform loading
                    modPtr->load();

                    // did we succeed?
                    if ( modPtr->errorMessage.isEmpty()) {
                        // mark as loaded
                        modPtr->loaded = true;
                        this->modList << modPtr;

                        // change colour & font in list
                        QFont font = item->font();
                        font.setBold( true );
                        item->setFont( font );
                        item->setBackground( QBrush( QColor( 0, 128, 0, 64 )));
                    } else {
                        // if we fail, make sure it will not be loaded again
                        // and show an error message
                        item->setIcon( QIcon( ":/icons/panic" ));

                        // cannot load bad module
                        this->itemLoadError( modPtr->errorMessage );
                    }
                }
            } else {
                // cannot load bad module
                this->itemLoadError( modPtr->errorMessage );
            }
            break;
        }
    }
}

/*
============
listWidgetAction
============
*/
void Sys_Module::listWidgetAction() {
    QListWidgetItem *item;

    // get current item
    item = this->modListWidget->currentItem();
    this->toggleFromList( item );
}
