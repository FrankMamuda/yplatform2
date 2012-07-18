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

#ifndef NF_STORAGEDRIVE_H
#define NF_STORAGEDRIVE_H

//
// includes
//
#include "module_global.h"

//
// class:StorageDrive
//
class StorageDrive : public QObject {
    Q_OBJECT
    Q_PROPERTY( QString udi READ udi WRITE setUdi )

public:
    explicit StorageDrive( Solid::Device );
    QList<Solid::Device> volumeList;
    void addChildVolume( Solid::Device );
    Solid::StorageDrive *drive() { return this->m_drivePtr; }
    QString udi() const { return this->m_udi; }

signals:
    
public slots:
    void setUdi( const QString &udi ) { this->m_udi = udi; }

private:
    Solid::StorageDrive *m_drivePtr;
    QString m_udi;
};

#endif // NF_STORAGEDRIVE_H
