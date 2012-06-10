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
#ifndef GUI_SETTINGS_H
#define GUI_SETTINGS_H

//
// includes
//
#include <QDialog>
#include <QTabWidget>
#include <QSpinBox>
#include <QCheckBox>
#include "../common/sys_cvarfunc.h"

//
// namespaces
//
namespace Ui {
    class Gui_Settings;
}

//
// class:pSettingsCvar
//
class pSettingsCvar : public QObject {
    Q_OBJECT
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_ENUMS( Types )

public:
    // currently supported types
    enum Types {
        CheckBox = 0,
        SpinBox
    };

    Types type () const { return this->m_type; }

    // constructor
    pSettingsCvar( pCvar *bCvarPtr, QObject *bObjPtr, pSettingsCvar::Types bType, QObject *parent );

    // set initial values from cvars
    void setState();

    // save value
    void save();

public slots:
    void setType( Types bType ) { this->m_type = bType; }

    // checkBox
    void stateChanged( int state );

    // spinBox
    void integerValueChanged( int integer );

private:
    Types m_type;

    // pointers to cvar and corresponding object
    pCvar *cvarPtr;
    QObject *objPtr;
};

//
// class:Gui_Settings
//
class Gui_Settings : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Settings dialog" )
    Q_PROPERTY( bool cvarsLocked READ cvarsLocked WRITE lockCvars )

public:
    explicit Gui_Settings( QWidget *parent = 0 );
    ~Gui_Settings();
    bool cvarsLocked() const { return this->m_cvarsLocked; }
    QTabWidget *settingsTabWidget;
    void addCvar( pCvar *cvarPtr, pSettingsCvar::Types type, QObject *objPtr ) {
        this->cvarList << new pSettingsCvar( cvarPtr, objPtr, type, qobject_cast<QObject*>( this ));
    }

signals:
    void updateModules();

protected:
    void changeEvent( QEvent *e );

private slots:
    void on_buttonAccept_clicked();
    void on_buttonClose_clicked();
    void lockCvars( bool lock = true ) { this->m_cvarsLocked = lock; }
    void saveCvars();
    void intializeCvars();

private:
    bool m_cvarsLocked;
    Ui::Gui_Settings *ui;
    bool s_fsDebug;

    // auto cvars
    QList <pSettingsCvar*>cvarList;
};

#endif // GUI_SETTINGS_H
