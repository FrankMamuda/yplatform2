/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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

#ifndef NF_DEVLIST_H
#define NF_DEVLIST_H

//
// includes
//
#include "module_global.h"
#include "nf_storagedrive.h"
#include "nf_scanner.h"
#include <QDialog>
#include <QListWidget>

//
// namespaces
//
namespace Ui {
    class DevList;
}

// metatypes
Q_DECLARE_METATYPE( Solid::Device )

//
// class:DevList
//
class DevList : public QDialog {
    Q_OBJECT

public:
    explicit DevList( QWidget *parent = 0 );
    ~DevList();
    void addTopDevice( StorageDrive * );
    void removeTopDevice( const QString & );
    void scan( Solid::Device );
    QStringList blockableList;
    void addBlockable( const QString & );

public slots:
    void init();
    void shutdown();

private slots:
    void on_devList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_scanButton_clicked();
    void scanComplete();
    void panic( const QString & );
    void setupDone( Solid::ErrorType, QVariant, const QString & );
    void on_removeButton_clicked();
    void on_addButton_clicked();
    void toggle();

private:
    Ui::DevList *ui;
    QList<Scanner*> scannerList;
};

#endif // NF_DEVLIST_H
