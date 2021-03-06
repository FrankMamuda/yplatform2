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
#include "gui_about.h"
#include "gui_license.h"
#include "ui_gui_about.h"
#include "../common/sys_shared.h"
#include "../common/sys_common.h"

/*
================
construct
================
*/
Gui_About::Gui_About( QWidget *parent ) :
    QDialog( parent ), m_ui( new Ui::Gui_About ) {
    m_ui->setupUi( this );

    m_ui->versionCommons->setText( QString( "%1" ).arg( Sys::Version ));
    m_ui->versionAPI->setText( this->tr( "API version: %1" ).arg( ModuleAPI::Version ));

    // this is a fixed frame
    this->setSizeGripEnabled( false );
}

/*
================
destruct
================
*/
Gui_About::~Gui_About() {
    delete m_ui;
}

/*
================
changeEvent
================
*/
void Gui_About::changeEvent( QEvent *e ) {
    QDialog::changeEvent( e );

    switch ( e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;

    default:
        break;
    }
}

/*
================
licenseButton->clicked
================
*/
void Gui_About::on_licenseButton_clicked() {
    Gui_License l;
    l.exec();
}
