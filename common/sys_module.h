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

#ifndef SYS_MODULE_H
#define SYS_MODULE_H

//
// includes
//
#include "sys_common.h"
#include "sys_modulefunc.h"
#include "../modules/mod_public.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QGridLayout>
#include <QBoxLayout>
#include <QKeyEvent>

//
// namespaces
//
namespace Module {
    class Sys_Module;
}

//
// class:pModuleWidget
//
class pModuleWidget : public QWidget {
protected:
    void keyPressEvent( QKeyEvent *event ) {
        // catch ESC
        if ( event->key() == Qt::Key_Escape ) {
            this->close();
            return;
        }

        QWidget::keyPressEvent( event );
    }
};

//
// class:Sys_Module
//
class Sys_Module : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Module handling subsystem" )
    Q_DECLARE_FLAGS ( PlatformFlags, ModuleAPI::Flags )
    Q_PROPERTY( PlatformFlags flags READ flags WRITE setFlags )

public slots:
    void init();
    void shutdown();
    void update();
    void updateCvar( const QString &cvar, const QString &stringValue );
    void toggleWidget();
    void preCacheModules();
    void reCacheModules();
    void populateListWidget();
    void createWidget();
    void destroyWidget();
    void setFlags( PlatformFlags flags ) { this->m_flags = flags; }

    // commands
    void load( const QStringList & );
    void unload( const QStringList & );

private:
    QList<pModule*>modList;
    pModuleWidget *modWidget;
    pModule *parseManifest( const QString &filename );
    QList<pModule*>preCachedList;
    QListWidget *modListWidget;
    void toggleFromList( QListWidgetItem *item );
    void itemLoadError( const QString &errorMessage );
    PlatformFlags m_flags;

    // widget items
    QGridLayout *mLayout;
    QBoxLayout *bLayout;
    QPushButton *loadButton;
    QPushButton *refreshButton;
    QPushButton *closeButton;

public:
    QVariant platformSyscalls( ModuleAPI::PlatformAPICalls callNum, const QVariantList &args );
    QVariant rendererSyscalls( RendererAPI::RendererAPICalls, const QVariantList &args );
    PlatformFlags flags() const { return this->m_flags; }

private slots:
    void listWidgetAction();
};

//
// externals
//
#ifndef MODULE_BUILD
extern class Sys_Module mod;
#endif

#endif // SYS_MODULE_H
