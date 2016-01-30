/*
===========================================================================
Copyright (C) 2011-2012 Edd 'Double Dee' Psycho

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
#include "r_texture.h"
#include "r_material.h"
#include "r_materialstage.h"
#include "r_main.h"
#include "r_cmd.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

/*
===================
construct
===================
*/
R_Material::R_Material( const QString &mtrName, QObject *parent ) : QObject( parent ) {
    this->setName( mtrName );
    this->validate();
}

/*
===================
destruct
===================
*/
R_Material::~R_Material() {
}

/*
===================
addStage
===================
*/
void R_Material::addStage( R_MaterialStage *stage ) {
    this->stageList << stage;
}

/*
===================
stage
===================
*/
R_MaterialStage *R_Material::stage( int index ) {
    if ( index == this->stageList.count()) {
        R_MaterialStage *stagePtr = new R_MaterialStage();
        this->stageList << stagePtr;
        return stagePtr;
    } else if ( index > this->stageList.count()) {
        com.error( StrSoftError + this->tr( "stage index '%1' out of range\n" ).arg( index ));
        return NULL;
    }
    return this->stageList.at( index );
}
