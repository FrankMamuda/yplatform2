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
#include "../gui/gui_settings.h"
#include "ui_gui_settings.h"
#include "../common/sys_cvar.h"
#include "../common/sys_common.h"
#include "../common/sys_filesystem.h"

//
// cvars
//
extern pCvar *gui_toolBarIconSize;
extern pCvar *fs_debug;
extern pCvar *fs_ignoreLinks;
extern pCvar *mod_extract;
extern pCvar *gui_restoreSize;

/*
================
construct
================
*/
Gui_Settings::Gui_Settings( QWidget *parent ) :QDialog( parent ), ui( new Ui::Gui_Settings ) {
    this->ui->setupUi( this );
    this->settingsTabWidget = this->ui->tabWidget;
}


/*
================
construct
================
*/
pSettingsCvar::pSettingsCvar( pCvar *bCvarPtr, QObject *bObjPtr, pSettingsCvar::Types bType, QObject *parent ) {
    QSpinBox *sPtr;
    QCheckBox *cPtr;

    // set data, type and object
    this->objPtr = bObjPtr;
    this->setType( bType );
    this->setParent( parent );
    this->cvarPtr = bCvarPtr;
    this->cvarPtr->resetTempString();

    // connect slots
    switch ( this->type()) {
    case CheckBox:
        cPtr = qobject_cast<QCheckBox*>( this->objPtr );
        cPtr->connect( cPtr, SIGNAL( stateChanged( int )), this, SLOT( stateChanged( int )));
        break;

    case SpinBox:
        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->connect( sPtr, SIGNAL( valueChanged( int )), this, SLOT( integerValueChanged( int )));
        break;

    default:
        com.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
setState
================
*/
void pSettingsCvar::setState() {
    QSpinBox *sPtr;
    QCheckBox *cPtr;

    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
        cPtr = qobject_cast<QCheckBox*>( this->objPtr );
        if ( this->cvarPtr->integer())
            cPtr->setCheckState( Qt::Checked );
        else
            cPtr->setCheckState( Qt::Unchecked );
        break;

    case SpinBox:
        sPtr = qobject_cast<QSpinBox*>( this->objPtr );
        sPtr->setValue( this->cvarPtr->integer());
        break;

    default:
        com.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
stateChanged
================
*/
void pSettingsCvar::stateChanged( int state ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->cvarPtr->set( true, pCvar::Temp );
    else
        this->cvarPtr->set( false, pCvar::Temp );
}

/*
================
integerValueChanged
================
*/
void pSettingsCvar::integerValueChanged( int integer ) {
    Gui_Settings *sParent = qobject_cast<Gui_Settings*>( this->parent());

    if ( sParent->cvarsLocked())
        return;

    this->cvarPtr->set( integer, pCvar::Temp );
}

/*
================
initializeCvars
================
*/
void Gui_Settings::intializeCvars() {
    // lock cvars
    this->lockCvars();

    // set default values
    this->addCvar( fs_debug, pSettingsCvar::CheckBox, this->ui->enableFsDebug );
    this->addCvar( fs_ignoreLinks, pSettingsCvar::CheckBox, this->ui->ignoreLinks );
    this->addCvar( mod_extract, pSettingsCvar::CheckBox, this->ui->extractModules );
    this->addCvar( gui_toolBarIconSize, pSettingsCvar::SpinBox, this->ui->iconSize );
    this->addCvar( gui_restoreSize, pSettingsCvar::CheckBox, this->ui->restoreSize );

#ifndef Q_OS_WIN
    this->ui->ignoreLinks->setDisabled( true );
#endif

    // unlock cvars
    this->lockCvars( false );

    // set state
    foreach ( pSettingsCvar *scPtr, this->cvarList )
        scPtr->setState();

    // emit signal for modules
    emit this->updateModules();
}

/*
================
destruct
================
*/
Gui_Settings::~Gui_Settings() {
    delete ui;
}

/*
================
changeEvent
================
*/
void Gui_Settings::changeEvent( QEvent *e ) {
    QDialog::changeEvent( e );

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
buttonAccept->clicked
================
*/
void Gui_Settings::on_buttonAccept_clicked() {
    this->saveCvars();
    this->accept();
}

/*
================
buttonClose->clicked
================
*/
void Gui_Settings::on_buttonClose_clicked() {
    this->reject();
}

/*
================
save
================
*/
void pSettingsCvar::save() {
    // set values to GUI
    switch ( this->type()) {
    case CheckBox:
    case SpinBox:
     this->cvarPtr->setTempString();
        break;

    default:
        com.error( StrSoftError + this->tr( "unknown type\n" ));
        return;
    }
}

/*
================
saveCvars
================
*/
void Gui_Settings::saveCvars() {
    foreach ( pSettingsCvar *scPtr, this->cvarList )
        scPtr->save();
}
