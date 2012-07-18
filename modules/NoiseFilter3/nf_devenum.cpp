/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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
#include "nf_main.h"
#include "nf_devlist.h"
#include "nf_devenum.h"
#include "nf_storagedrive.h"
#include "../mod_trap.h"
#include "../../common/sys_common.h"

//
// classes
//
class DevEnum de;

//
// cvars
//
extern mCvar *nf_blockableItems;

/*
================
init
================
*/
void DevEnum::init() {
    QList<Solid::Device> dList;
    QList<Solid::Device> vList;
    StorageDrive *sdPtr;

    // create notifier
    this->notifierPtr = Solid::DeviceNotifier::instance();

    // create deviceList
    this->dlPtr = new DevList();

    // get storage drive and volume list
    dList = Solid::Device::listFromType( Solid::DeviceInterface::StorageDrive );

    // check the whole list
    foreach ( Solid::Device drive, dList ) {
        // get child volumes
        vList = this->getChildVolumes( drive );

        // create new storage drive container
        sdPtr = new StorageDrive( drive );

        // add volumes if any
        if ( !vList.isEmpty()) {
            foreach ( Solid::Device volume, vList )
                sdPtr->addChildVolume( volume );
        }

        // add this entry to the global list
        this->driveList << sdPtr;

        // add device to gui
        this->dlPtr->addTopDevice( sdPtr );
    }

    // make sure to monitor new devices
    this->connect( this->notifierPtr, SIGNAL( deviceAdded( QString )), this, SLOT( onDeviceAdded( QString )));
    this->connect( this->notifierPtr, SIGNAL( deviceRemoved( QString )), this, SLOT( onDeviceRemoved( QString )));

    // set up blockables
    this->dlPtr->init();

    // make sure to update on cvar changes
    this->connect( nf_blockableItems, SIGNAL( valueChanged( QString, QString )), this->dlPtr, SLOT( init()));

    // setup toolbar
    this->tbPtr = new QToolBar();
    this->tbPtr->setIconSize( QSize( 48, 48 ));
    this->tbPtr->setMovable( false );
    this->tbPtr->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    this->tbPtr->addAction( QIcon( ":/icons/noise_filter_48" ), "Noise Filter", this->dlPtr, SLOT( toggle()));
    gui.addToolBar( this->tbPtr );
}

/*
================
shutdown
================
*/
void DevEnum::shutdown() {
    // stop monitoring
    this->disconnect( this->notifierPtr, SIGNAL( deviceAdded( QString )), this, SLOT( onDeviceAdded( QString )));
    this->disconnect( this->notifierPtr, SIGNAL( deviceRemoved( QString )), this, SLOT( onDeviceRemoved( QString )));
    this->disconnect( nf_blockableItems, SIGNAL( valueChanged( QString, QString )), this->dlPtr, SLOT( init()));

    // remove toolbar
    gui.removeToolBar( this->tbPtr );
    delete this->tbPtr;

    // get rid of devlist
    this->dlPtr->shutdown();
    delete this->dlPtr;

    // clean up
    foreach ( StorageDrive *sdPtr, this->driveList ) {
        sdPtr->volumeList.clear();
        delete sdPtr;
    }
    this->driveList.clear();
}

/*
================
getChildVolumes
================
*/
QList<Solid::Device> DevEnum::getChildVolumes( const Solid::Device &dev ) {
    QList<Solid::Device> volumeList;
    QList<Solid::Device> outList;

    // get volume list
    volumeList = Solid::Device::listFromType( Solid::DeviceInterface::StorageVolume );

    // compare udis
    foreach ( Solid::Device volume, volumeList ) {
        if ( !QString::compare( this->getParent( volume ).udi(), dev.udi()))
            outList << volume;
    }

    return outList;
}

/*
================
getParentDrive
================
*/
Solid::StorageDrive *DevEnum::getParentDrive( const Solid::Device &dev ) {
    Solid::Device parent = dev;

    while (( parent = parent.parent()).isValid()) {
        if ( parent.is<Solid::StorageDrive>())
            return parent.as<Solid::StorageDrive>();
    }
    return NULL;
}

/*
================
getParent
================
*/
Solid::Device DevEnum::getParent( const Solid::Device &dev ) {
    Solid::Device parent = dev;

    while (( parent = parent.parent()).isValid()) {
        if ( parent.is<Solid::StorageDrive>())
            return parent;
    }
    return Solid::Device();
}

/*
================
onDeviceAdded
================
*/
void DevEnum::onDeviceAdded( const QString &devName ) {
    Solid::Device dev( devName );
    StorageDrive *sdPtr;

    if ( dev.is<Solid::StorageVolume>()) {
        // get child volumes
        Solid::Device parent = this->getParent( dev );

        // match drive
        foreach ( StorageDrive *drivePtr, this->driveList ) {
            if ( !QString::compare( drivePtr->udi(), parent.udi())) {
                drivePtr->addChildVolume( dev );
                return;
            }
        }

        // nothing? - create a new storage drive container
        sdPtr = new StorageDrive( parent );

        // add the volume
        sdPtr->addChildVolume( dev );

        // add this entry to the global list
        this->driveList << sdPtr;

        // add device to gui
        this->dlPtr->addTopDevice( sdPtr );

        // scan IMMEDIATELY, since this is a new device that
        // might contain unbearable NOISE!!!
        this->dlPtr->scan( dev );
    }
}

/*
================
onDeviceRemoved
================
*/
void DevEnum::onDeviceRemoved( const QString &devName ) {
    Solid::Device dev( devName );

    foreach ( StorageDrive *sdPtr, this->driveList ) {
        if ( !QString::compare( sdPtr->udi(), dev.udi())) {
            this->driveList.removeOne( sdPtr );
            this->dlPtr->removeTopDevice( sdPtr->udi());
        }
    }
}
