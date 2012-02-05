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

//
// namespaces
//
namespace Ui {
    class Gui_Settings;
}

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
    bool cvarsLocked() const { return this->m_cvarsLocked; };
    QTabWidget *settingsTabWidget;

signals:
    void updateModules();

protected:
    void changeEvent( QEvent *e );

private slots:
    void on_buttonAccept_clicked();
    void on_buttonClose_clicked();
    void lockCvars( bool lock = true ) { this->m_cvarsLocked = lock; };
    void on_iconSize_valueChanged( int );
    void on_extractModules_stateChanged( int );
    void on_enableFsDebug_stateChanged( int );
    void on_ignoreLinks_stateChanged( int );
    void saveCvars();
    void intializeCvars();

    void on_restoreSize_stateChanged(int arg1);

private:
    bool m_cvarsLocked;
    Ui::Gui_Settings *ui;
    bool s_fsDebug;
    bool s_modExtract;
    unsigned int s_guiIconSize;
    bool s_restoreSize;
    bool s_ignoreLinks;
};

#endif // GUI_SETTINGS_H
