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
    bool foundModule = false;

    // just allocate a new pointer with the current filename
    modPtr = new pModule( filename );

    // read buffer
    QByteArray buffer = fs.readFile( filename, Sys_Filesystem::Silent );

    // failsafe
    if ( buffer.isNull()) {
        modPtr->setErrorMessage( this->tr( "Invalid manifest" ));
        return modPtr;
    }

    //
    // parse document
    //
    manifest.setContent( buffer );
    QDomNode moduleNode = manifest.firstChild();
    while ( !moduleNode.isNull()) {
        if ( moduleNode.isElement()) {
            QDomElement moduleElement = moduleNode.toElement();

            // check element name
            if ( QString::compare( moduleElement.tagName(), "module" )) {
                modPtr->setErrorMessage( this->tr( "Manifest syntax error: expected <module>" ));
                return modPtr;
            } else
                foundModule = true;

            // entries must have names
            if ( moduleElement.hasAttribute( "name" )) {
                QString modName = moduleElement.attribute( "name" );

                // set the parsed name
                modPtr->setName( modName );
            } else {
                modPtr->setErrorMessage( this->tr( "Module missing name" ));
                return modPtr;
            }

            if ( moduleElement.hasAttribute( "type" )) {
                QString type = moduleElement.attribute( "type" );

                if ( !QString::compare( type, "module" ))
                    modPtr->setType( pModule::Module );
                else if ( !QString::compare( type, "renderer" )) {
                    modPtr->setType( pModule::Renderer );
                } else
                    modPtr->setErrorMessage( this->tr( "Unknown module type" ));
            }

            QDomNode infoNode = moduleElement.firstChild();
            while ( !infoNode.isNull()) {
                if ( infoNode.isElement()) {
                    QDomElement infoElement = infoNode.toElement();

                    // check element name
                    if ( !QString::compare( infoElement.tagName(), "description", Qt::CaseInsensitive ))
                        modPtr->setDescription( infoElement.text());
                    else if ( !QString::compare( infoElement.tagName(), "icon", Qt::CaseInsensitive ))
                        modPtr->setIcon( infoElement.text());
                    else if ( !QString::compare( infoElement.tagName(), "api", Qt::CaseInsensitive )) {
                        modPtr->setApiVersion( infoElement.text().toInt());
                        if ( modPtr->apiVersion() > ModuleAPI::Version ) {
                            modPtr->setErrorMessage( this->tr( "API version mismatch - %1, expected less or equal to %2" ).arg( modPtr->apiVersion()).arg( ModuleAPI::Version ));
                            return modPtr;
                        }
                    } else if ( !QString::compare( infoElement.tagName(), "filename", Qt::CaseInsensitive ))
                        modPtr->setFilename( infoElement.text());
                    else if ( !QString::compare( infoElement.tagName(), "version", Qt::CaseInsensitive ))
                        modPtr->setVersionString( infoElement.text());
                    else {
                        modPtr->setErrorMessage( this->tr( "Manifest syntax error: expected module info element, found <%1>" ).arg( infoElement.tagName()));
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
        modPtr->setErrorMessage( this->tr( "Missing module entry in manifest" ));

    // clear buffer
    buffer.clear();

    // return the new module
    return modPtr;
}

/*
============
load
============
*/
void Sys_Module::load( const QStringList &args ) {
    bool found = false;

    if ( args.isEmpty()) {
        com.print( Sys::cYellow + this->tr( "usage: ^2mod_load ^3[^2module name^3]\n" ));
        return;
    }

    // loading actual modules, not manifest filenames
    QList<QListWidgetItem*>list = this->modListWidget->findItems( args.first(), Qt::MatchExactly );
    foreach ( QListWidgetItem *item, list ) {
        foreach ( pModule *modPtr, this->preCachedList ) {
            if ( !QString::compare( args.first(), modPtr->name())) {
                if ( !modPtr->isLoaded()) {
                    this->toggleFromList( item );
                    found = true;
                    break;
                } else {
                    com.print( StrWarn + this->tr( "module \"%1\" has already been loaded\n" ).arg( modPtr->name()));
                    return;
                }
            }
        }
    }

    if ( !found ) {
        com.print( StrWarn + this->tr( "could not find module \"%1\"\n" ).arg( args.first()));
        this->itemLoadError( this->tr( "Could not find module \"%1\"\n" ).arg( args.first()));
    }
}

/*
============
unload
============
*/
void Sys_Module::unload( const QStringList &args ) {
    if ( args.isEmpty()) {
        com.print( Sys::cYellow + this->tr( "usage: ^2mod_unload ^3[^2module name^3]\n" ));
        return;
    }

    // unloading actual modules, not manifest filenames
    QList<QListWidgetItem*>list = this->modListWidget->findItems( args.first(), Qt::MatchExactly );
    foreach ( QListWidgetItem *item, list ) {
        foreach ( pModule *modPtr, this->preCachedList ) {
            if ( !QString::compare( args.first(), modPtr->name() )) {
                if ( modPtr->isLoaded())
                    this->toggleFromList( item );
                else
                    com.print( StrWarn + this->tr( "module \"%1\" has already been unloaded\n" ).arg( modPtr->name()));
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
    // begin frame on each \er window first
    foreach ( pModule *modPtr, this->modList ) {
        if ( modPtr->type() == pModule::Renderer )
            modPtr->call( RendererAPI::BeginFrame );
    }

    // do module updates here so that we can actually
    // render something if needed
    foreach ( pModule *modPtr, this->modList )
        modPtr->update();

    // end frame
    foreach ( pModule *modPtr, this->modList ) {
        if ( modPtr->type() == pModule::Renderer )
            modPtr->call( RendererAPI::EndFrame );
    }
}

/*
============
init
============
*/
void Sys_Module::init() {
    // add commands
    cmd.add( "mod_load", loadCmd, this->tr( "load module by name" ));
    cmd.add( "mod_unload", unloadCmd, this->tr( "unload module" ));

    // init cvars
    mod_extract = cv.create( "mod_extract", "1", pCvar::Archive, this->tr( "toggle module copying from packages" ));

    // reset
    this->setFlags( ModuleAPI::NoFlags );

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
    cmd.remove( "mod_load" );
    cmd.remove( "mod_unload" );

    // destroy widget
    this->destroyWidget();

    // unload all modules
    foreach( pModule *modPtr, this->modList )
        modPtr->unload();

    // delete all precached modules
    foreach( pModule *modPtr, this->preCachedList )
        delete modPtr;
    this->modList.clear();
}

/*
=================
platformSyscalls
=================
*/
QVariant Sys_Module::platformSyscalls( ModuleAPI::PlatformAPICalls callNum, const QVariantList &args ) {
    pCvar *cvarPtr;

    switch ( callNum ) {

    //
    // commons
    //
    case ModuleAPI::ComPrint:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "ComPrint [message]\n" ));
            return false;
        }

        com.print( args.first().toString());
        return true;

    case ModuleAPI::ComError:
        if ( args.count() != 2 )  {
            com.error( Sys_Common::SoftError, this->tr( "ComError [type] [message]\n" ));
            return false;
        }

        com.error( static_cast<Sys_Common::ErrorTypes>( args.first().toInt()), args.at( 1 ).toString());
        return true;

    case ModuleAPI::ComMilliseconds:
        return com.milliseconds();

        //
        // filesystem
        //
    case ModuleAPI::FsOpen:
        if ( args.count() != 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsOpen [mode] [filename] [&handle] (flags)\n" ));
            return false;
        }
        return static_cast<int>( fs.open( static_cast<pFile::OpenModes>( args.at( 0 ).toInt()), args.at( 1 ).toString(),
                                          *( args.at( 2 ).value<fileHandle_t*>()), static_cast<Sys_Filesystem::OpenFlags>( args.at( 3 ).toInt())));

    case ModuleAPI::FsClose:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsClose [handle] (flags)\n" ));
            return false;
        }

        fs.close( static_cast<fileHandle_t>( args.at( 0 ).toInt()), static_cast<Sys_Filesystem::OpenFlags>( args.at( 1 ).toInt()));
        return true;

    case ModuleAPI::FsCloseByName:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsCloseByName [filename] (flags)\n" ));
            return false;
        }

        fs.close( args.first().toString(), static_cast<Sys_Filesystem::OpenFlags>( args.at( 1 ).toInt()));
        return true;

    case ModuleAPI::FsExists:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsExists [path] (flags)\n" ));
            return false;
        }
        return fs.exists( args.first().toString(), static_cast<Sys_Filesystem::OpenFlags>( args.at( 1 ).toInt()));

    case ModuleAPI::FsRead:
        if ( args.count() != 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsRead [&buffer] [len] [handle] (flags)\n" ));
            return false;
        }
        return static_cast<int>( fs.read( args.at( 0 ).value<byte*>(), args.at( 1 ).toInt(), static_cast<fileHandle_t>( args.at( 2 ).toInt()), static_cast<Sys_Filesystem::OpenFlags>( args.at( 3 ).toInt())));

    case ModuleAPI::FsWrite:
        // we cannot allow inappropriate calls to fs write! -> fatal error
        if ( args.count() != 3 ) {
            com.error( Sys_Common::FatalError, this->tr( "FsWrite [buffer] [handle] (flags)\n" ));
            return false;
        }

        return static_cast<int>( fs.write( args.first().toByteArray(), static_cast<fileHandle_t>( args.at( 1 ).toInt()), static_cast<Sys_Filesystem::OpenFlags>( args.at( 2 ).toInt())));

    case ModuleAPI::FsSeek:
        if ( args.count() != 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsSeek [handle] [offset] (flags) (seek mode)\n" ));
            return false;
        }

        return fs.seek( static_cast<fileHandle_t>( args.first().toInt()), args.at( 1 ).toInt(), static_cast<Sys_Filesystem::OpenFlags>( args.at( 2 ).toInt()), static_cast<Sys_Filesystem::SeekModes>( args.at( 3 ).toInt()));

    case ModuleAPI::FsTouch:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsTouch [filename] (flags)\n" ));
            return false;
        }

        fs.touch( args.first().toString(), static_cast<Sys_Filesystem::OpenFlags>( args.at( 1 ).toInt()));
        return true;

    case ModuleAPI::FsReadFile:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsReadFile [filename] (flags)\n" ));
            return false;
        }
        return fs.readFile( args.first().toString(), static_cast<Sys_Filesystem::OpenFlags>( args.at( 1 ).toInt()));

    case ModuleAPI::FsPrint:
        if ( args.count() != 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsPrint [handle] [msg] (flags)\n" ));
            return false;
        }
        fs.print( static_cast<fileHandle_t>( args.first().toInt()), args.at( 1 ).toString(), static_cast<Sys_Filesystem::OpenFlags>( args.at( 2 ).toInt()));
        return true;

    case ModuleAPI::FsExtract:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsExtract [filename]\n" ));
            return false;
        }
        return fs.extract( args.first().toString());

    case ModuleAPI::FsList:
        if ( args.count() != 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "FsList [directory] (filter) (mode)\n" ));
            return false;
        }
        return fs.list( args.first().toString(), args.at( 1 ).toRegExp(), static_cast<Sys_Filesystem::ListModes>( args.at( 2 ).toInt()));

        //
        // command subsystem
        //
    case ModuleAPI::CmdAdd:
        if ( args.count() != 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "CmdAdd [cmdName] [cmd] (description)\n" ));
            return false;
        }
        cmd.add( args.first().toString(), args.at( 1 ).value<cmdCommand_t>(), args.at( 2 ).toString());
        return true;

    case ModuleAPI::CmdAddScripted:
        if ( args.count() != 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "CmdAddScripted [cmdName] [function] (description)\n" ));
            return false;
        }
        cmd.add( args.first().toString(), args.at( 1 ).value<QScriptValue>(), args.at( 2 ).toString());
        return true;

    case ModuleAPI::CmdRemove:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "CmdRemove [cmdName]\n" ));
            return false;
        }
        cmd.remove( args.first().toString());
        return true;

    case ModuleAPI::CmdExecute:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "CmdExecute [command string]\n" ));
            return false;
        }
        cmd.execute( args.first().toString(), Sys_Cmd::Delayed );
        return true;

        //
        // cvar subsystem
        //
    case ModuleAPI::CvarCreate:
        if ( args.count() != 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "CvarCreate [name] [string] (flags) (description)\n" ));
            return false;
        }
        if ( cv.create( args.first().toString(), args.at( 1 ).toString(), static_cast<pCvar::Flags>( args.at( 2 ).toInt()), args.at( 3 ).toString(), true ) != NULL )
            return true;

        return false;

    case ModuleAPI::CvarSet:
        if ( args.count() != 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "CvarSet [name] [value] (force)\n" ));
            return false;
        }
        cvarPtr = cv.find( args.first().toString());
        if ( cvarPtr != NULL ) {
            cvarPtr->set( args.at( 1 ).toString(), static_cast<pCvar::AccessFlags>( args.at( 2 ).toInt()));
            return true;
        }
        return false;

    case ModuleAPI::CvarGet:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "CvarGet [name]\n" ));
            return false;
        }
        cvarPtr = cv.find( args.first().toString());
        if ( cvarPtr != NULL )
            return cvarPtr->string();
        return QObject::trUtf8( "undefined" );

    case ModuleAPI::CvarReset:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "CvarReset [name]\n" ));
            return false;
        }
        cmd.execute( QString( "cv_reset %1" ).arg( args.first().toString()));
        return true;

        //
        // applet
        //
    case ModuleAPI::AppShutdown:
        // shut down, no questions asked
        m.shutdown();
        return true;

        //
        // gui
        //
    case ModuleAPI::GuiRaise:
        com.gui()->show();
        return true;

    case ModuleAPI::GuiHide:
        com.gui()->hide();
        return true;

    case ModuleAPI::GuiCreateSystray:
        com.gui()->createSystemTray();
        return true;

    case ModuleAPI::GuiRemoveSystray:
        com.gui()->removeSystemTray();
        return true;

    case ModuleAPI::GuiRemoveAction:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiRemoveAction [name]\n" ));
            return false;
        }
        com.gui()->removeAction( static_cast<ModuleAPI::ToolBarActions>( args.first().toInt()));
        return true;

    case ModuleAPI::GuiAddToolBar:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiAddToolbar [toolBarPtr]\n" ));
            return false;
        }
        com.gui()->addToolBar( reinterpret_cast<QToolBar*>( args.at( 0 ).value<void*>()));
        return true;

    case ModuleAPI::GuiRemoveToolBar:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiRemoveToolBar [toolBarPtr]\n" ));
            return false;
        }
        com.gui()->removeToolBar( reinterpret_cast<QToolBar*>( args.at( 0 ).value<void*>()));
        return true;

    case ModuleAPI::GuiRemoveMainToolBar:
        com.gui()->removeMainToolBar();
        return true;

    case ModuleAPI::GuiAddTab:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiAddTab [widgetPtr] [name] (icon)\n" ));
            return false;
        }
        com.gui()->addTabExt( Gui_Main::MainWindow, args.at( 0 ).value<QWidget*>(), args.at( 1 ).toString(), args.at( 2 ).toString());
        return true;

    case ModuleAPI::GuiRemoveTab:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiRemoveTab [name]\n" ));
            return false;
        }
        com.gui()->removeTabExt( Gui_Main::MainWindow, args.first().toString());
        return true;

    case ModuleAPI::GuiSetActiveTab:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiSetActiveTab [name]\n" ));
            return false;
        }
        com.gui()->setActiveTab( args.first().toString());
        return true;

    case ModuleAPI::GuiSetConsoleState:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiSetConsoleState [state]\n" ));
            return false;
        }
        com.gui()->setConsoleState( static_cast<ModuleAPI::ConsoleState>( args.first().toInt()));
        return true;

    case ModuleAPI::GuiAddSettingsTab:
        if ( args.count() != 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiAddSettingsTab [widgetPtr] [name] (icon)\n" ));
            return false;
        }
        com.gui()->addTabExt( Gui_Main::Settings, args.at( 0 ).value<QWidget*>(), args.at( 1 ).toString(), args.at( 2 ).toString());
        return true;

    case ModuleAPI::GuiRemoveSettingsTab:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "GuiRemoveSettingsTab [name]\n" ));
            return false;
        }
        com.gui()->removeTabExt( Gui_Main::Settings, args.first().toString());
        return true;

    case ModuleAPI::GuiShowTabWidget:
        com.gui()->showTabWidget();
        return true;

    case ModuleAPI::GuiHideTabWidget:
        com.gui()->hideTabWidget();
        return true;

        //
        // platform
        //
    case ModuleAPI::PlatformFlags:
        return static_cast<int>( this->flags());

        //
        // renderer
        //
    case ModuleAPI::RendererKeyEvent:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "RendererKeyEvent [type] [key]\n" ));
            return false;
        }

        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::KeyEvent, args.at( 0 ), args.at( 1 ));
        }
        return true;

    case ModuleAPI::RendererMouseEvent:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "RendererMouseEvent [type] [key]\n" ));
            return false;
        }
        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::MouseEvent, args.at( 0 ), args.at( 1 ));
        }
        return true;

    case ModuleAPI::RendererMouseMotion:
        if ( args.count() != 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "RendererMouseMotion [x] [y]\n" ));
            return false;
        }
        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::MouseMotion, args.at( 0 ), args.at( 1 ));
        }
        return true;

    case ModuleAPI::RendererWheelEvent:
        if ( args.count() != 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "RendererWheelEvent [delta]\n" ));
            return false;
        }
        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::WheelEvent, args.at( 0 ));
        }
        return true;

    default:
        // we cannot accept calls from a faulty module - might damage filesystem or else
        // abort!
        com.error( StrFatalError + this->tr( "unknown callNum '%1'\n" ).arg( callNum ));
        return false;
    }
    return false;
}

/*
=================
rendererSyscalls
=================
*/
QVariant Sys_Module::rendererSyscalls( RendererAPI::RendererAPICalls callNum, const QVariantList &args ) {
    // yes, we support multiple renderers
    foreach ( pModule *modPtr, this->modList ) {
        if ( modPtr->type() == pModule::Renderer ) {
            switch ( callNum ) {
            case RendererAPI::LoadMaterial:
                if ( args.count() != 1 ) {
                    com.error( Sys_Common::SoftError, this->tr( "RendererLoadMaterial [name]\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::LoadMaterial, args.at( 0 ));

            case RendererAPI::DrawMaterial:
                if ( args.count() != 5 ) {
                    com.error( Sys_Common::SoftError, this->tr( "RendererDrawMaterial [position] [size] [handle]\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::DrawMaterial, args.at( 0 ), args.at( 1 ), args.at( 2 ), args.at( 3 ), args.at( 4 ));

            case RendererAPI::DrawText:
                if ( args.count() != 8 ) {
                    com.error( Sys_Common::SoftError, this->tr( "DrawText [x] [y] [fontPtr] [string] (colour)\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::DrawText, args.at( 0 ), args.at( 1 ), args.at( 2 ), args.at( 3 ), args.at( 4 ), args.at( 5 ), args.at( 6 ), args.at( 7 ));

            case RendererAPI::SetColour:
                if ( args.count() != 4 ) {
                    com.error( Sys_Common::SoftError, this->tr( "RendererSetColour [red] [green] [blue] [alpha]\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::SetColour, args.at( 0 ), args.at( 1 ), args.at( 2 ), args.at( 3 ));

            case RendererAPI::Raise:
                return modPtr->call( RendererAPI::Raise );

            case RendererAPI::Hide:
                return modPtr->call( RendererAPI::Hide );

            case RendererAPI::Reload:
                return modPtr->call( RendererAPI::Reload );

            case RendererAPI::SetWindowTitle:
                if ( args.count() != 1 ) {
                    com.error( Sys_Common::SoftError, this->tr( "SetWindowTitle [string])\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::SetWindowTitle, args.at( 0 ));

            default:
                // abort
                com.error( StrFatalError + this->tr( "unknown callNum '%1'\n" ).arg( callNum ));
                return false;
            }

        }
    }
    return true;
}

/*
============
updateCvar
============
*/
void Sys_Module::updateCvar( const QString &cvar, const QString &stringValue ) {
    foreach ( pModule *modPtr, this->modList ) {
        if ( modPtr->type() == pModule::Renderer )
            modPtr->call( RendererAPI::UpdateCvar, cvar, stringValue );
        else
            modPtr->call( ModuleAPI::UpdateCvar, cvar, stringValue );
    }
}

/*
============
moduleSort (sorting: valid first, a-z descending)
============
*/
bool moduleSort( const pModule *m1, const pModule *m2 ) {
    if ( m1->getErrorMessage().isEmpty() && m2->getErrorMessage().isEmpty())
        return m1->name().toLower() < m2->name().toLower();
    else if ( m1->getErrorMessage().isEmpty() && !m2->getErrorMessage().isEmpty())
        return true;
    else if ( !m1->getErrorMessage().isEmpty() && m2->getErrorMessage().isEmpty())
        return false;
    else if ( !m1->getErrorMessage().isEmpty() && !m2->getErrorMessage().isEmpty())
        return m1->name().toLower() < m2->name().toLower();

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
    com.print( StrMsg + this->tr( "recaching modules\n" ));

    // rather dirrty, but a working solution
    // removing those manifests already loaded
    foreach ( pModule *modPtr, this->preCachedList ) {
        foreach ( pModule *modPtrLoaded, this->modList ) {
            if ( !QString::compare( modPtr->name(), modPtrLoaded->name()))
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
    QStringList mList = fs.list( "modules/", mFilter, Sys_Filesystem::ListFiles );
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
        if ( modPtr->getErrorMessage().isEmpty()) {
            QIcon tempIcon;

            if ( !modPtr->icon().isEmpty()) {
                if ( modPtr->icon().startsWith( ":" )) {
                    tempIcon = QIcon( modPtr->icon() );
                } else {
                    QByteArray buffer = fs.readFile( fs.defaultExtension( modPtr->icon(), ".png" ), Sys_Filesystem::Silent );

                    // any icon?
                    if ( !buffer.isEmpty()) {
                        QPixmap pixMap;
                        pixMap.loadFromData( buffer );
                        tempIcon = QIcon( pixMap );
                    }
                    buffer.clear();
                }
            }

            if ( tempIcon.isNull())
                tempIcon = QIcon( ":/icons/module_128" );


            // generate filename
            QString filename = QString( "modules/%1%2_%3_%4.%5" )
                    .arg( LIBRARY_PREFIX )
                    .arg( modPtr->filename())
                    .arg( LIBRARY_SUFFIX )
                    .arg( ARCH_STRING )
                    .arg( LIBRARY_EXT );


            // perliminary check (we don't care if it's in a pack or not)
            if ( !fs.exists( filename ))
                tempIcon = QIcon( ":/icons/panic" );

            QListWidgetItem *moduleItem = new QListWidgetItem( tempIcon, modPtr->name());
            if ( modPtr->isLoaded()) {
                // change colour & font in list
                QFont font = moduleItem->font();
                font.setBold( true );
                moduleItem->setFont( font );
                moduleItem->setBackground( QBrush( QColor( 0, 128, 0, 64 )));
            }

            this->modListWidget->addItem( moduleItem );
        } else
            this->modListWidget->addItem( new QListWidgetItem( QIcon( ":/icons/panic" ), modPtr->name()));
    }
}

/*
============
createWidget
============
*/
void Sys_Module::createWidget() {
    // create list widget
    this->modListWidget = new QListWidget( com.gui());
    this->modListWidget->setIconSize( QSize( 32, 32 ));
    this->modListWidget->setAlternatingRowColors( true );

    // add entries
    this->populateListWidget();

    // connect
    this->connect( this->modListWidget, SIGNAL( itemDoubleClicked( QListWidgetItem* )), this, SLOT( listWidgetAction()));

    // simple widget
    this->modWidget = new pModuleWidget();
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

    // connect load button
    this->loadButton = new QPushButton( QIcon( ":/icons/exec" ), this->tr( "Load/Unload" ));
    this->connect( this->loadButton, SIGNAL( clicked()), this, SLOT( listWidgetAction()));
    this->bLayout->addWidget( this->loadButton );

    // connect refresh button
    this->refreshButton = new QPushButton( QIcon( ":/icons/refresh" ),this->tr( "Refresh" ));
    this->connect( this->refreshButton, SIGNAL( clicked()), this, SLOT( reCacheModules()));
    this->bLayout->addWidget( this->refreshButton );

    // connect close button
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
        if ( this->modWidget->isVisible() && this->modWidget->hasFocus()) {
            this->modWidget->hide();
        } else {
            // position it properly
            this->modWidget->move( QPoint( com.gui()->pos().x() + com.gui()->width()/2 - this->modWidget->width()/2, com.gui()->pos().y()));
            this->modWidget->showNormal();
            this->modWidget->raise();
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
    msgBox.setWindowTitle( this->tr( "Error" ));
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
        if ( !QString::compare( modPtr->name(), item->text())) {
            // is it errorous already?
            if ( modPtr->getErrorMessage().isEmpty()) {
                // unload if loaded
                if ( modPtr->isLoaded() && modPtr->type() == pModule::Module ) {
                    // perform unloading, mark as unloaded
                    modPtr->unload();
                    modPtr->setLoaded( false );
                    this->modList.removeOne( modPtr );

                    // change colour & font in list
                    QFont font = item->font();
                    font.setBold( false );
                    item->setFont( font );
                    item->setBackground( QBrush( QColor( 0, 0, 0, 0 )));
                    break;
                } else if ( modPtr->isLoaded() && modPtr->type() == pModule::Renderer ) {
                    // just hide the renderer, we cannot reload it
                    RendererAPI::WindowState state = static_cast<RendererAPI::WindowState>( modPtr->call( RendererAPI::State ).toInt());
                    if ( state == RendererAPI::Raised ) {
                        // change font in list
                        QFont font = item->font();
                        font.setItalic( true );
                        item->setFont( font );
                        item->setBackground( QBrush( QColor( 255, 250, 0, 64 )));

                        // hide
                        modPtr->call( RendererAPI::Hide );
                    } else {
                        // change font in list
                        QFont font = item->font();
                        font.setItalic( false );
                        item->setFont( font );
                        item->setBackground( QBrush( QColor( 0, 128, 0, 64 )));

                        // show
                        modPtr->call( RendererAPI::Raise );
                    }
                } else {
                    // perform loading
                    modPtr->load();

                    // did we succeed?
                    if ( modPtr->getErrorMessage().isEmpty()) {
                        // mark as loaded
                        modPtr->setLoaded( true );
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
                        this->itemLoadError( modPtr->getErrorMessage());
                    }
                }
            } else {
                // cannot load bad module
                this->itemLoadError( modPtr->getErrorMessage());
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
