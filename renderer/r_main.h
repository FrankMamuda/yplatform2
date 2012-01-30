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

#ifndef R_MAIN_H
#define R_MAIN_H

//
// includes
//
#include "r_shared.h"
#include "r_image.h"
#include "r_material.h"
#include "r_settings.h"

//
// namespaces
//
namespace Renderer {
    static const int FuncTableSize = 1024;
    static const int NumFuncTables = 6;
}

//
// class:R_Main
//
class R_Main : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Renderer core" )
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )
    Q_PROPERTY( float time READ time WRITE setTime )
    Q_ENUMS( FuncTables )

public:
    QList <R_Image*>imageList;
    QStringList missingList;
    QList <R_Material*>mtrList;
    QList <R_MaterialStage*>mtrStageList;
    imgHandle_t defaultImage;
    mtrHandle_t platformLogo;
    imgHandle_t loadImage( const QString &, R_Image::ClampModes = R_Image::Repeat );
    mtrHandle_t loadMaterial( const QString & );
    QStringList extensionList;

    // function tables
    float funcTable[Renderer::NumFuncTables][Renderer::FuncTableSize];
    double degreesToRadians( double );

    // property getters
    bool hasInitialized() const { return this->m_initialized; }
    float time() const { return this->m_time; }

signals:

public slots:
    void init( bool reload = false );
    void beginFrame();
    void endFrame();
    void shutdown( bool reload = false );
    void listImages();
    void listMaterials();

    // property setters
    void setInitialized( bool intialized = true ) { this->m_initialized = intialized; }
    void setTime( float time ) { this->m_time = time; }

private:
    bool m_initialized;
    float m_time;
    R_Settings *settingsWidget;
};

//
// externals
//
#ifdef R_BUILD
extern class R_Main m;
#endif

#endif // R_MAIN_H
