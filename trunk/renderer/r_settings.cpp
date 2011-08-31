/*
===========================================================================
Copyright (C) 2011 Edd 'Double Dee' Psycho

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
#include "r_settings.h"
#include "r_glimp.h"
#include "ui_r_settings.h"

//
// cvars
//
extern mCvar *r_screenMode;
extern mCvar *r_adjustScreen;

/*
================
construct
================
*/
R_Settings::R_Settings( QWidget *parent ) : QWidget( parent ), ui( new Ui::R_Settings ) {
    int y;

    // setup widget
    ui->setupUi( this );
    this->ui->versionLabel->setText( Renderer::Title + " " + Renderer::Version );

    // add screen modes
    for ( y = 0; y < Renderer::NumScreenModes; y++ ) {
        QString mode = QString( "%1x%2" ).arg( Renderer::HorizontalScreenModes[y] ).arg( Renderer::VerticalScreenModes[y] );
        this->ui->screenMode->addItem( mode, y );
    }

    // init cvars
    this->intializeCvars();
}

/*
================
intializeCvars
================
*/
void R_Settings::intializeCvars() {
    // lock cvars
    this->lockCvars();

    // set default values
    this->s_adjust = r_adjustScreen->integer();
    if ( this->s_adjust )
        this->ui->adjustScreen->setCheckState( Qt::Checked );
    else
        this->ui->adjustScreen->setCheckState( Qt::Unchecked );

    // set default values
    this->s_mode = r_screenMode->integer();
    this->ui->screenMode->setCurrentIndex( this->s_mode );

    // unlock cvars
    this->lockCvars( false );
}

/*
================
destruct
================
*/
R_Settings::~R_Settings() {
    delete ui;
}

/*
================
changeEvent
================
*/
void R_Settings::changeEvent( QEvent *e ) {
    QWidget::changeEvent( e );

    switch ( e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/*
================
saveCvars
================
*/
void R_Settings::saveCvars() {
    r_adjustScreen->set( this->s_adjust );
    r_screenMode->set( this->s_mode );
}

/*
================
adjustScreen->stateChanged
================
*/
void R_Settings::on_adjustScreen_stateChanged( int state ) {
    if ( this->cvarsLocked())
        return;

    if ( state == Qt::Checked )
        this->s_adjust = true;
    else
        this->s_adjust = false;
}

/*
================
screenMode->currentIndexChanged
================
*/
void R_Settings::on_screenMode_currentIndexChanged( int index ) {
    if ( this->cvarsLocked())
        return;

    this->s_mode = index;
}
