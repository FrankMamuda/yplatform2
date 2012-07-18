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

#ifndef NF_DEVICEENUM_H
#define NF_DEVICEENUM_H

//
// includes
//
#include "module_global.h"
#include "nf_storagedrive.h"
#include "nf_devlist.h"

//
// class:DevEnum
//
class DevEnum : public QObject {
    Q_OBJECT

private:
    Solid::StorageDrive *getParentDrive( const Solid::Device & );
    Solid::Device getParent( const Solid::Device & );
    QList<Solid::Device> getChildVolumes( const Solid::Device & );
    QList<StorageDrive*> driveList;

public slots:
    void init();
    void shutdown();
    void onDeviceAdded( const QString & );
    void onDeviceRemoved( const QString & );

private:
    Solid::DeviceNotifier *notifierPtr;
    DevList *dlPtr;
    QToolBar *tbPtr;
};

//
// externals
//
extern class DevEnum de;

#endif // NF_DEVICEENUM_H
