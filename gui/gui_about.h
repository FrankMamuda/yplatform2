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

#ifndef GUI_ABOUT_H
#define GUI_ABOUT_H

//
// includes
//
#include <QDialog>

//
// namespaces
//
namespace Ui {
    class Gui_About;
}

//
// class:Gui_About
//
class Gui_About : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "About dialog" )

public:
    Gui_About( QWidget *parent = 0 );
    ~Gui_About();

protected:
    void changeEvent( QEvent *e );

private:
    Ui::Gui_About *m_ui;

private slots:
    void on_licenseButton_clicked();
};

#endif // GUI_ABOUT_H
