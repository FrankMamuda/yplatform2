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
    long len;
    bool foundModule = false;

    // just allocate a new pointer with the current filename
    modPtr = new pModule( filename );

    // read buffer
    byte *buffer;
    len = fs.readFile( filename, &buffer, Sys_Filesystem::Silent );

    // failsafe
    if ( len <= 0 ) {
        modPtr->setErrorMessage( this->tr( "Invalid manifest" ));
        return modPtr;
    }

    //
    // parse document
    //
    manifest.setContent( QByteArray( reinterpret_cast<const char*>( buffer ), len ));
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
    bool found = false;

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
            if ( !QString::compare( mName, modPtr->name())) {
                if ( !modPtr->isLoaded()) {
                    this->toggleFromList( item );
                    found = true;
                    break;
                } else
                    com.print( this->tr( "^3Sys_Module::load: module \"%1\" has already been loaded\n" ).arg( modPtr->name()));
            }
        }
    }

    if ( !found )
        com.print( this->tr( "^3Sys_Module::load: could not find module \"%1\"\n" ).arg( mName ));
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
            if ( !QString::compare( mName, modPtr->name() )) {
                if ( modPtr->isLoaded())
                    this->toggleFromList( item );
                else
                    com.print( this->tr( "^3Sys_Module::load: module \"%1\" has already been unloaded\n" ).arg( modPtr->name()));
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
    // begin frame on each renderer window first
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
    mod_extract = cv.create( "mod_extract", "1", pCvar::Archive, "toggle module copying from packages" );

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
intptr_t Sys_Module::platformSyscalls( ModuleAPI::PlatformAPICalls callNum, int numArgs, intptr_t *args ) {
    switch ( callNum ) {

    //
    // commons
    //
    case ModuleAPI::ComPrint:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: ComPrint [message]\n" ));
            return false;
        }

        com.print( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::ComError:
        if ( numArgs < 2 )  {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: ComError [type] [message]\n" ));
            return false;
        }

        com.error( static_cast<Sys_Common::ErrorTypes>( args[0] ), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )));
        break;

    case ModuleAPI::ComMilliseconds:
        return static_cast<intptr_t>( com.milliseconds());

        //
        // filesystem
        //
    case ModuleAPI::FsOpen:
        if ( numArgs < 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsOpen [mode] [filename] (&handle) (flags)\n" ));
            return false;
        }
        return static_cast<intptr_t>( fs.open( static_cast<pFile::OpenModes>( args[0] ), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )), *( reinterpret_cast<fileHandle_t*>( args[2] )), static_cast<Sys_Filesystem::OpenFlags>( args[3] )));

    case ModuleAPI::FsClose:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsClose [handle] (flags)\n" ));
            return false;
        }

        fs.close( static_cast<fileHandle_t>( args[0] ), static_cast<Sys_Filesystem::OpenFlags>( args[1] ));
        break;

    case ModuleAPI::FsCloseByName:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsCloseByName [filename] (flags)\n" ));
            return false;
        }

        fs.close( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), static_cast<Sys_Filesystem::OpenFlags>( args[1] ));
        break;

    case ModuleAPI::FsExists:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsExists [path] (flags)\n" ));
            return false;
        }
        return static_cast<intptr_t>( fs.exists( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), static_cast<Sys_Filesystem::OpenFlags>( args[1] )));

    case ModuleAPI::FsRead:
        if ( numArgs < 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsRead [&buffer] [len] [handle] (flags)\n" ));
            return false;
        }

        return static_cast<intptr_t>( fs.read( *( reinterpret_cast<byte**>( args[0] )), static_cast<int>( args[1] ), static_cast<fileHandle_t>( args[2] ), static_cast<Sys_Filesystem::OpenFlags>( args[3] )));

    case ModuleAPI::FsWrite:
        // we cannot allow inappropriate calls to fs write! -> fatal error
        if ( numArgs < 4 ) {
            com.error( Sys_Common::FatalError, this->tr( "platformSyscalls: FsWrite [buffer] [len] [handle] (flags)\n" ));
            return false;
        }

        return static_cast<intptr_t>( fs.write( reinterpret_cast<const byte*>( args[0] ), static_cast<int>( args[1] ), static_cast<fileHandle_t>( args[2] ), static_cast<Sys_Filesystem::OpenFlags>( args[3] )));

    case ModuleAPI::FsSeek:
        if ( numArgs < 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsSeek [handle] [offset] (flags) (seek mode)\n" ));
            return false;
        }

        return static_cast<intptr_t>( fs.seek( static_cast<fileHandle_t>( args[0] ), static_cast<long>( args[1] ), static_cast<Sys_Filesystem::OpenFlags>( args[2] ), static_cast<Sys_Filesystem::SeekModes>( args[3] )));

    case ModuleAPI::FsTouch:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsTouch [filename] (flags)\n" ));
            return false;
        }

        fs.touch( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), static_cast<Sys_Filesystem::OpenFlags>( args[1] ));
        break;

    case ModuleAPI::FsReadFile:
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsReadFile [filename] [&buffer] (flags)\n" ));
            return false;
        }

        return static_cast<intptr_t>( fs.readFile( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), reinterpret_cast<byte**>( args[1] ), static_cast<Sys_Filesystem::OpenFlags>( args[2] )));

    case ModuleAPI::FsPrint:
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsPrint [handle] [msg] (flags)\n" ));
            return false;
        }

        fs.print( static_cast<fileHandle_t>( args[0] ), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )), static_cast<Sys_Filesystem::OpenFlags>( args[2] ));
        break;

    case ModuleAPI::FsFreeFile:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsFreeFile [filename]\n" ));
            return false;
        }

        fs.freeFile( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::FsExtract:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsExtract [filename]\n" ));
            return false;
        }

        return static_cast<intptr_t>( fs.extract( QString::fromLatin1( reinterpret_cast<const char*>( args[0] ))));

    case ModuleAPI::FsList:
    {
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: FsList [directory] [buffer] [len] (filter) (mode)\n" ));
            return false;
        }

        // beware: don't use semicolon in dirname... but why should you in the first place?
        QString fileList = fs.list( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), *( reinterpret_cast<const QRegExp*>( args[3] )), static_cast<Sys_Filesystem::ListModes>( args[4] )).join( ";" );
        qstrncpy( reinterpret_cast<char*>( args[1] ), fileList.toLatin1().constData(), static_cast<unsigned int>( args[2] ));
        return true;
    }
        break;

        //
        // command subsystem
        //
    case ModuleAPI::CmdAdd:
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CmdAdd [cmdName] [cmd] (description)\n" ));
            return false;
        }

        cmd.add( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), reinterpret_cast<cmdCommand_t>( args[1] ), QString::fromLatin1( reinterpret_cast<const char*>( args[2] )));
        break;

    case ModuleAPI::CmdRemove:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CmdRemove [cmdName]\n" ));
            return false;
        }

        cmd.remove( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::CmdArgc:
        return static_cast<intptr_t>( cmd.argc());

    case ModuleAPI::CmdArgv:
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CmdArgv [argId] [buffer] [len]\n" ));
            return false;
        }
        qstrncpy( reinterpret_cast<char*>( args[1] ), cmd.argv( static_cast<int>( args[0] )).toLatin1().constData(), static_cast<int>( args[2] ));
        return true;


    case ModuleAPI::CmdExecute:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CmdExecute [command string]\n" ));
            return false;
        }

        return cmd.execute( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));

        //
        // cvar subsystem
        //
    case ModuleAPI::CvarCreate:
        if ( numArgs < 4 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CvarCreate [name] [string] (flags) (description)\n" ));
            return false;
        }
        if ( cv.create( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )), static_cast<pCvar::Flags>( args[2] ), QString::fromLatin1( reinterpret_cast<const char*>( args[3] )), true ) != NULL )
            return true;
        else
            return false;

    case ModuleAPI::CvarSet:
    {
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CvarSet [name] [value] (force)\n" ));
            return false;
        }
        pCvar *cvarPtr = cv.find( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        if ( cvarPtr != NULL ) {
            cvarPtr->set( QString::fromLatin1( reinterpret_cast<const char*>( args[1] )), static_cast<bool>( args[2] ));
            return true;
        } else
            return false;
    }
        break;

    case ModuleAPI::CvarGet:
    {
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CvarGet [name] [buffer] [len]\n" ));
            return false;
        }
        pCvar *cvarPtr = cv.find(( const char * )args[0] );
        if ( cvarPtr != NULL ) {
            qstrncpy( reinterpret_cast<char*>( args[1] ), cvarPtr->string().toLatin1().constData(), static_cast<int>( args[2] ));
            return true;
        } else
            return false;
    }
        break;

    case ModuleAPI::CvarReset:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: CvarReset [name]\n" ));
            return false;
        }

        cmd.execute( QString( "cv_reset %1" ).arg( QString::fromLatin1( reinterpret_cast<const char*>( args[0] ))));
        break;

        //
        // applet
        //
    case ModuleAPI::AppShutdown:
        // shut down, no questions asked
        m.shutdown();
        break;

        //
        // gui
        //
    case ModuleAPI::GuiRaise:
        com.gui()->show();
        break;

    case ModuleAPI::GuiHide:
        com.gui()->hide();
        break;

    case ModuleAPI::GuiCreateSystray:
        com.gui()->createSystemTray();
        break;

    case ModuleAPI::GuiRemoveSystray:
        com.gui()->removeSystemTray();
        break;

    case ModuleAPI::GuiAddAction:
        if ( numArgs < 3 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiAddAction [name] [icon] [callback]\n" ));
            return false;
        }

        com.gui()->addToolBarAction( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )),  reinterpret_cast<cmdCommand_t>( args[2] ));
        break;

    case ModuleAPI::GuiRemoveAction:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiRemoveAction [name]\n" ));
            return false;
        }
        com.gui()->removeAction( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::GuiAddTab:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiAddTab [widgetPtr] [name] (icon)\n" ));
            return false;
        }
        com.gui()->addTabExt( Gui_Main::MainWindow, reinterpret_cast<QWidget*>( args[0] ), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )), QString::fromLatin1( reinterpret_cast<const char*>( args[2] )));
        break;

    case ModuleAPI::GuiRemoveTab:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiRemoveTab [name]\n" ));
            return false;
        }
        com.gui()->removeTabExt( Gui_Main::MainWindow, QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::GuiSetActiveTab:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiSetActiveTab [name]\n" ));
            return false;
        }
        com.gui()->setActiveTab( QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::GuiSetConsoleState:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiSetConsoleState [state]\n" ));
            return false;
        }
        com.gui()->setConsoleState( static_cast<ModuleAPI::ConsoleState>( args[0] ));
        break;

    case ModuleAPI::GuiAddSettingsTab:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiAddSettingsTab [widgetPtr] [name] (icon)\n" ));
            return false;
        }
        com.gui()->addTabExt( Gui_Main::Settings, reinterpret_cast<QWidget*>( args[0] ), QString::fromLatin1( reinterpret_cast<const char*>( args[1] )), QString::fromLatin1( reinterpret_cast<const char*>( args[2] )));
        break;

    case ModuleAPI::GuiRemoveSettingsTab:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: GuiRemoveSettingsTab [name]\n" ));
            return false;
        }
        com.gui()->removeTabExt( Gui_Main::Settings, QString::fromLatin1( reinterpret_cast<const char*>( args[0] )));
        break;

    case ModuleAPI::RendererKeyEvent:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: RendererKeyEvent [type] [key]\n" ));
            return false;
        }

        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::KeyEvent, args[0], args[1] );
        }
        break;

    case ModuleAPI::RendererMouseEvent:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: RendererMouseEvent [type] [key]\n" ));
            return false;
        }

        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::MouseEvent, args[0], args[1] );
        }
        break;

    case ModuleAPI::RendererMouseMotion:
        if ( numArgs < 2 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: RendererMouseMotion [x] [y]\n" ));
            return false;
        }

        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::MouseMotion, args[0], args[1] );
        }
        break;

    case ModuleAPI::RendererWheelEvent:
        if ( numArgs < 1 ) {
            com.error( Sys_Common::SoftError, this->tr( "platformSyscalls: RendererWheelEvent [delta]\n" ));
            return false;
        }

        foreach ( pModule *modPtr, this->modList ) {
            if ( modPtr->type() != pModule::Renderer )
                modPtr->call( ModuleAPI::WheelEvent, args[0] );
        }
        break;

    default:
        // we cannot accept calls from a faulty module - might damage filesystem or else
        // abort!
        com.error( Sys_Common::FatalError, this->tr( "platformSyscalls: unknown callNum '%1'\n" ).arg( callNum ));
        return false;
    }
    return true;
}

/*
=================
rendererSyscalls
=================
*/
intptr_t Sys_Module::rendererSyscalls( RendererAPI::RendererAPICalls callNum, int numArgs, intptr_t *args ) {
    // yes, we support multiple renderers
    foreach ( pModule *modPtr, this->modList ) {
        if ( modPtr->type() == pModule::Renderer ) {
            switch ( callNum ) {
            case RendererAPI::LoadMaterial:
                if ( numArgs < 1 ) {
                    com.error( Sys_Common::SoftError, this->tr( "rendererSyscalls: RendererLoadMaterial [name]\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::LoadMaterial, args[0] );

            case RendererAPI::DrawMaterial:
                if ( numArgs < 5 ) {
                    com.error( Sys_Common::SoftError, this->tr( "rendererSyscalls: RendererDrawMaterial [position] [size] [handle]\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::DrawMaterial, args[0], args[1], args[2], args[3], args[4] );

            case RendererAPI::DrawText:
                if ( numArgs < 4 ) {
                    com.error( Sys_Common::SoftError, this->tr( "rendererSyscalls: RendererLoadFont [x] [y] [fontPtr] [string] (colour)\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::DrawText, args[0], args[1], args[2], args[3],
                                    args[4], args[5], args[6], args[7] );

            case RendererAPI::SetColour:
                if ( numArgs < 4 ) {
                    com.error( Sys_Common::SoftError, this->tr( "rendererSyscalls: RendererSetColour [red] [green] [blue] [alpha]\n" ));
                    return false;
                }
                return modPtr->call( RendererAPI::SetColour, args[0], args[1], args[2], args[3] );

            case RendererAPI::Raise:
                return modPtr->call( RendererAPI::Raise );

            case RendererAPI::Hide:
                return modPtr->call( RendererAPI::Hide );

            default:
                // abort
                com.error( Sys_Common::FatalError, this->tr( "rendererSyscalls: unknown callNum '%1'\n" ).arg( callNum ));
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
            modPtr->call( RendererAPI::UpdateCvar, reinterpret_cast<intptr_t>( cvar.toLatin1().constData()), reinterpret_cast<intptr_t>( stringValue.toLatin1().constData()));
        else
            modPtr->call( ModuleAPI::UpdateCvar, reinterpret_cast<intptr_t>( cvar.toLatin1().constData()), reinterpret_cast<intptr_t>( stringValue.toLatin1().constData()));
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
    com.print( this->tr( "^2Sys_Module::reCacheModules: ^5recaching modules\n" ));

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
                    byte *buffer;
                    long len = fs.readFile( modPtr->icon(), &buffer, Sys_Filesystem::Silent );

                    // any icon?
                    if ( len > 0 ) {
                        QPixmap pixMap;
                        pixMap.loadFromData( QByteArray( reinterpret_cast<const char*>( buffer ), len ));
                        tempIcon = QIcon( pixMap );
                    }
                }
            }

            if ( tempIcon.isNull())
                tempIcon = QIcon( ":/icons/module_128" );

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
            this->modWidget->move( QPoint( com.gui()->pos().x() + com.gui()->width()/2 - this->modWidget->width()/2, com.gui()->pos().y()));
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
        if ( !QString::compare( modPtr->name(), item->text())) {
            // is it errorous already?
            if ( modPtr->getErrorMessage().isEmpty()) {
                // unload if loaded
                if ( modPtr->isLoaded() ) {
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
