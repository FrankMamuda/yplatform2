/*
===========================================================================
Copyright (C) 2011-2012 Edd 'Double Dee' Psycho

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

#ifndef R_MATERIAL_H
#define R_MATERIAL_H

//
// includes
//
#include "r_shared.h"
#include "r_materialstage.h"

//
// classes
//
class R_MaterialStage;

//
// class:R_Material
//
class R_Material : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Material" )
    Q_DISABLE_COPY( R_Material )
    Q_PROPERTY( bool valid READ isValid WRITE validate )
    Q_PROPERTY( QString name READ name WRITE setName )

public:
    explicit R_Material( const QString &mtrName, QObject *parent = 0 );
    ~R_Material();
    QList <R_MaterialStage*>stageList;

    // property getters
    bool isValid() const { return this->m_valid; }
    QString name() const { return this->m_name; }

signals:

public slots:
    void addStage( R_MaterialStage * );
    R_MaterialStage *stage( int index );

    // property setters
    void validate( bool valid = true ) { this->m_valid = valid; }
    void setName( const QString &name ) { this->m_name = name; }

private:
    // properties
    bool m_valid;
    QString m_name;
};

#endif // R_MATERIAL_H
