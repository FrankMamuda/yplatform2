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

#ifndef GUI_PASSWORD_H
#define GUI_PASSWORD_H

//
// includes
//
#include <QDialog>
#include "ui_gui_password.h"

//
// namespaces
//
namespace Ui {
    class Gui_Password;
}

//
// class:Gui_Password
//
class Gui_Password : public QDialog, public Ui::Gui_Password {
    Q_OBJECT
    Q_DISABLE_COPY( Gui_Password )

public:
    explicit Gui_Password( QWidget *parent = 0 );
    virtual ~Gui_Password();

protected:
    virtual void changeEvent( QEvent *e );

public:
    Ui::Gui_Password *ui;
};

#endif // GUI_PASSWORD_H
