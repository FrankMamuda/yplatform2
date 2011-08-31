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

#ifndef R_SETTINGS_H
#define R_SETTINGS_H

//
// includes
//
#include <QWidget>
#include "../modules/mod_trap.h"

//
// namesapces
//
namespace Ui {
    class R_Settings;
}

//
// class:R_Settings
//
class R_Settings : public QWidget {
    Q_OBJECT
    Q_CLASSINFO( "description", "Renderer settings dialog" )
    Q_PROPERTY( bool cvarsLocked READ cvarsLocked WRITE lockCvars )

public:
    explicit R_Settings( QWidget *parent = 0 );
    ~R_Settings();
    bool cvarsLocked() const { return this->m_cvarsLocked; };

public slots:
    void saveCvars();
    void intializeCvars();

protected:
    void changeEvent( QEvent *e );

private:
    Ui::R_Settings *ui;
    bool s_adjust;
    int s_mode;
    bool m_cvarsLocked;

private slots:
    void lockCvars( bool lock = true ) { this->m_cvarsLocked = lock; };
    void on_adjustScreen_stateChanged( int );
    void on_screenMode_currentIndexChanged(int index);
};

#endif // R_SETTINGS_H
