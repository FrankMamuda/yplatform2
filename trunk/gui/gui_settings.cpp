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
initializeCvars
================
*/
void Gui_Settings::intializeCvars() {
    // lock cvars
    this->lockCvars();

    // set default values
    this->s_fsDebug = fs_debug->integer();
    if ( this->s_fsDebug )
        this->ui->enableFsDebug->setCheckState( Qt::Checked );
    else
        this->ui->enableFsDebug->setCheckState( Qt::Unchecked );

    // set default values
    this->s_ignoreLinks = fs_ignoreLinks->integer();
    if ( this->s_ignoreLinks )
        this->ui->ignoreLinks->setCheckState( Qt::Checked );
    else
        this->ui->ignoreLinks->setCheckState( Qt::Unchecked );
#ifndef Q_OS_WIN
    this->ui->ignoreLinks->setDisabled( true );
#endif

    // set default values
    this->s_modExtract = mod_extract->integer();
    if ( this->s_modExtract )
        this->ui->extractModules->setCheckState( Qt::Checked );
    else
        this->ui->extractModules->setCheckState( Qt::Unchecked );

    // set default values
    this->s_guiIconSize = gui_toolBarIconSize->integer();
    this->ui->iconSize->setValue( this->s_guiIconSize );

    // set default values
    this->s_restoreSize = gui_restoreSize->integer();
    if ( this->s_restoreSize )
        this->ui->restoreSize->setCheckState( Qt::Checked );
    else
        this->ui->restoreSize->setCheckState( Qt::Unchecked );

    // unlock cvars
    this->lockCvars( false );

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
saveCvars
================
*/
void Gui_Settings::saveCvars() {
    gui_toolBarIconSize->set( static_cast<int>( this->s_guiIconSize ));
    mod_extract->set( this->s_modExtract );
    fs_debug->set( this->s_fsDebug );
    fs_ignoreLinks->set( this->s_ignoreLinks );
    gui_restoreSize->set( this->s_restoreSize );
}

/*
================
iconSize->valueChanged
================
*/
void Gui_Settings::on_iconSize_valueChanged( int value ) {
    if ( this->cvarsLocked())
        return;

    this->s_guiIconSize = value;
}

/*
================
extractModules->stateChanged
================
*/
void Gui_Settings::on_extractModules_stateChanged( int state ) {
    if ( this->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->s_modExtract = true;
    else
        this->s_modExtract = false;
}

/*
================
enableFsDebug->stateChanged
================
*/
void Gui_Settings::on_enableFsDebug_stateChanged( int state ) {
    if ( this->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->s_fsDebug = true;
    else
        this->s_fsDebug = false;
}

/*
================
ignoreLinks->stateChanged
================
*/
void Gui_Settings::on_ignoreLinks_stateChanged( int state ) {
    if ( this->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->s_ignoreLinks = true;
    else
        this->s_ignoreLinks = false;
}

/*
================
restoreSize->stateChanged
================
*/
void Gui_Settings::on_restoreSize_stateChanged( int state ){
    if ( this->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->s_restoreSize = true;
    else
        this->s_restoreSize = false;
}
