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
#include "nf_devlist.h"
#include "ui_nf_devlist.h"
#include "nf_storagedrive.h"
#include "nf_scanner.h"
#include "nf_devenum.h"
#include "../mod_trap.h"
#include "../mod_cvarfunc.h"
#include "../../common/sys_common.h"
#include <QtDBus>

//
// cvars
//
extern mCvar *nf_blockableItems;

/*
================
construct
================
*/
DevList::DevList( QWidget *parent ) : QDialog( parent ), ui( new Ui::DevList ) {
    ui->setupUi( this );

    // set defaults
    this->ui->devList->expandAll();
    this->ui->scanButton->setDisabled( true );
}

/*
================
init
================
*/
void DevList::init() {
    // init blockables
    this->ui->blockableList->clear();
    this->blockableList.clear();
    this->blockableList << nf_blockableItems->string().split( ';' );
    foreach ( QString str, this->blockableList )
        this->ui->blockableList->addItem( new QListWidgetItem( str ));
}


/*
================
shutdown
================
*/
void DevList::shutdown() {
    // clear thread storage
    foreach ( Scanner *scanPtr, this->scannerList ) {
        scanPtr->terminate();
        delete scanPtr;
    }
    this->scannerList.clear();
}

/*
================
destruct
================
*/
DevList::~DevList() {
    delete ui;
}

/*
================
removeTopDevice
================
*/
void DevList::removeTopDevice( const QString &udi ) {
    int y;
    QTreeWidgetItem *itemPtr = NULL;

    for ( y = 0; y < this->ui->devList->topLevelItemCount(); y++ ) {
        itemPtr = this->ui->devList->topLevelItem( y );

        if ( !QString::compare( itemPtr->data( 0, Qt::UserRole ).toString(), udi )) {
            this->ui->devList->takeTopLevelItem( y );
            break;
        }
    }
}

/*
================
addTopDevice
================
*/
void DevList::addTopDevice( StorageDrive *drivePtr ) {
    QTreeWidgetItem *itemPtr = new QTreeWidgetItem();

    // check for volumes
    if ( drivePtr->volumeList.isEmpty())
        return;

    // currently we handle only optical discs, harddrives and
    switch ( drivePtr->drive()->driveType()) {
    case Solid::StorageDrive::HardDisk:
        if ( drivePtr->drive()->isRemovable())
            itemPtr->setIcon( 0, QIcon( ":/icons/drive_rm_48" ));
        else
            itemPtr->setIcon( 0, QIcon( ":/icons/drive_hd_48" ));

        break;

    case Solid::StorageDrive::CompactFlash:
    case Solid::StorageDrive::MemoryStick:
    case Solid::StorageDrive::SmartMedia:
    case Solid::StorageDrive::SdMmc:
    case Solid::StorageDrive::Xd:
        itemPtr->setIcon( 0, QIcon( ":/icons/drive_rm_48" ));
        break;

    case Solid::StorageDrive::CdromDrive:
        itemPtr->setIcon( 0, QIcon( ":/icons/drive_cd_48" ));
        break;

    default:
        return;
    }

    // set name and add to list
    itemPtr->setText( 0, drivePtr->udi());
    itemPtr->setData( 0, Qt::UserRole, drivePtr->udi());
    this->ui->devList->addTopLevelItem( itemPtr );

    // add volumes
    foreach ( Solid::Device volume, drivePtr->volumeList ) {
        QTreeWidgetItem *childPtr = new QTreeWidgetItem();
        Solid::StorageVolume *volumePtr = volume.as<Solid::StorageVolume>();

        // skip swap partitions, etc.
        if ( volumePtr->isIgnored())
            continue;

        // determine icon
        switch ( drivePtr->drive()->driveType()) {
        case Solid::StorageDrive::HardDisk:
            if ( drivePtr->drive()->isRemovable())
                childPtr->setIcon( 0, QIcon( ":/icons/media_rm_48" ));
            else
                childPtr->setIcon( 0, QIcon( ":/icons/media_hd_48" ));
            break;

        case Solid::StorageDrive::CompactFlash:
        case Solid::StorageDrive::SdMmc:
        case Solid::StorageDrive::MemoryStick:
        case Solid::StorageDrive::SmartMedia:
        case Solid::StorageDrive::Xd:
            childPtr->setIcon( 0, QIcon( ":/icons/media_sd_48" ));
            break;

        case Solid::StorageDrive::CdromDrive:
            childPtr->setIcon( 0, QIcon( ":/icons/media_cd_48" ));
            break;

        default:
            return;
        }

        // set name
        if ( volumePtr->label().isEmpty())
            childPtr->setText( 0, volumePtr->uuid());

        else
            childPtr->setText( 0, volumePtr->label());

        // store volume info and add to list
        childPtr->setData( 0, Qt::UserRole, volume.udi());
        itemPtr->addChild( childPtr );
    }

    // reexpand list
    this->ui->devList->expandAll();
}

/*
================
devList->currentItemChanged
================
*/
void DevList::on_devList_currentItemChanged( QTreeWidgetItem *itemPtr, QTreeWidgetItem * ) {
    // has parent -> assuming is volume
    if ( itemPtr->parent() != NULL )
        this->ui->scanButton->setEnabled( true );
    else
        this->ui->scanButton->setDisabled( true );
}

/*
================
scan
================
*/
void DevList::scan( Solid::Device dev ) {
    Solid::StorageAccess *accessPtr = dev.as<Solid::StorageAccess>();
    QString name;
    Solid::StorageVolume *volumePtr = dev.as<Solid::StorageVolume>();

    // set name
    if ( volumePtr->label().isEmpty())
        name = volumePtr->uuid();
    else
        name = volumePtr->label();

    // mount device if it is not accessible yet
    this->connect( accessPtr, SIGNAL( setupDone( Solid::ErrorType, QVariant, QString )), this, SLOT( setupDone( Solid::ErrorType, QVariant, QString)));
    if ( !accessPtr->isAccessible()) {
        if ( accessPtr->setup())
            com.print( StrMsg + this->tr( "attempting to mount \"%1\"\n" ).arg( name ));
        else {
            com.error( StrSoftError + this->tr( "mount failed on \"%1\"\n" ).arg( name ));
            return;
        }
    } else
        this->setupDone( Solid::NoError, QVariant(), dev.udi());
}

/*
================
setupDone
================
*/
void DevList::setupDone( Solid::ErrorType error, QVariant errorData, const QString &udi ) {
    Solid::Device dev( udi );
    QString name;
    Solid::StorageVolume *volumePtr = dev.as<Solid::StorageVolume>();
    Solid::StorageAccess *accessPtr = dev.as<Solid::StorageAccess>();

    // set name
    if ( volumePtr->label().isEmpty())
        name = volumePtr->uuid();
    else
        name = volumePtr->label();

    // any errors?
    if ( error && errorData.isValid()) {
        com.error( StrSoftError + this->tr( "mount failed on \"%1\"\n" ).arg( name ));
        return;
    }

    // announce scan
    com.print( StrMsg + this->tr( "scanning \"%1\"\n" ).arg( accessPtr->filePath()));

    // create scanner
    Scanner *scanPtr = new Scanner();
    scanPtr->setPath( accessPtr->filePath());
    scanPtr->setBlockables( this->blockableList );
    this->connect( scanPtr, SIGNAL( finished()), this, SLOT( scanComplete()));
    this->connect( scanPtr, SIGNAL( panic( QString )), this, SLOT( panic( QString )));
    scanPtr->start();
}

/*
================
scanButton->clicked
================
*/
void DevList::on_scanButton_clicked() {
    Solid::Device dev( this->ui->devList->currentItem()->data( 0, Qt::UserRole ).toString());
    this->scan( dev );
}

/*
================
scanComplete
================
*/
void DevList::scanComplete() {
    // announce
    com.print( StrMsg + this->tr( "scan complete\n" ));
}

/*
================
panic
================
*/
void DevList::panic( const QString &filePath ) {
    // announce
    com.error( StrSoftError + this->tr( "NOISE detected in \"%1\"\n" ).arg( filePath ));

    // lock screen IMMEDIATELY
    QDBusInterface lock( "org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver" );
    lock.call( "Lock" );

    // display error
    QMessageBox::critical( 0, this->tr( "NoiseFilter" ), QObject::trUtf8( "NOISE detected in \"%1\"\n" ).arg( filePath ));

    // show gui
    gui.raise();
}

/*
================
removeButton->clicked
================
*/
void DevList::on_removeButton_clicked() {
    if ( this->ui->blockableList->currentItem() != NULL ) {
        this->blockableList.removeOne( this->ui->blockableList->currentItem()->text());
        this->ui->blockableList->takeItem( this->ui->blockableList->currentRow());
        nf_blockableItems->set( this->blockableList.join( ";" ));
    }
}

/*
================
addButton->clicked
================
*/
void DevList::on_addButton_clicked() {
    this->addBlockable( this->ui->blockable->text());
    this->ui->blockable->clear();
}

/*
================
addBlockable
================
*/
void DevList::addBlockable( const QString &text ) {
    if ( !text.isEmpty()) {
        if ( !QString::compare( text, "rock", Qt::CaseInsensitive )) {
            QMessageBox::critical( 0, this->tr( "NoiseFilter" ), this->tr( "You cannot stop The ROCK\n" ));
            return;
        }

        foreach ( QString str, this->blockableList ) {
            if ( !QString::compare( str, text ))
                return;
        }

        this->blockableList << text;
        nf_blockableItems->set( this->blockableList.join( ";" ));
    }
}

/*
================
toggle
================
*/
void DevList::toggle() {
    if ( this->isVisible())
        this->hide();
    else
        this->show();
}
