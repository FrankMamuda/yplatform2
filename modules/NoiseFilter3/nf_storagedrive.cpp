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

/*
================
construct
================
*/
StorageDrive::StorageDrive( Solid::Device dev ) {
    // create a pointer to the drive
    this->m_drivePtr = dev.as<Solid::StorageDrive>();
    this->setUdi( dev.udi());
}

/*
================
addChildVolume
================
*/
void StorageDrive::addChildVolume( Solid::Device dev ) {
    // add the new device
    this->volumeList.append( dev );
}
