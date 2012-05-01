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

#ifndef R_CMD_H
#define R_CMD_H

//
// includes
//
#include "r_shared.h"
#include "r_materialstage.h"

//
// namespaces
//
namespace Renderer {
    static const int MaterialVertexes = 4;
    static const int TriSurfVertexes = 6;
    static const int TriSurf[TriSurfVertexes] = { 3, 0, 2, 2, 0, 1 };
}

//
// class:R_Cmd
//
class R_Cmd : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Renderer command subsystem" )
    Q_ENUMS( SrcBlend::Modes )
    Q_ENUMS( DstBlend::Modes )

public:
    struct SrcBlend {
        enum Modes {
            Zero = 1,
            One,
            Colour,
            MinusColour,
            SrcAlpha,
            MinusSrcAlpha,
            DstAlpha,
            MinusDstAlpha,
            AlphaSaturate
        };
    };
    struct DstBlend {
        enum Modes {
            Zero = 1,
            One,
            Colour,
            MinusColour,
            SrcAlpha,
            MinusSrcAlpha,
            DstAlpha,
            MinusDstAlpha
        };
    };

    void drawImage( float x, float y, float w, float h, float s1, float t1, float s2, float t2, imgHandle_t handle );
    void drawImage( float x, float y, float w, float h, imgHandle_t handle ) { this->drawImage( x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, handle ); }
    void drawMaterial( float x, float y, float w, float h, mtrHandle_t handle );
    void setColour( float r, float g, float b, float a = 1.0f, bool directly = false  );
    void setColour( const QColor = QColor::fromRgbF( 0.0f, 0.0f, 0.0f, 1.0f ), bool directly = false );
    void setColour( const Renderer::Vec4D colour, bool directly = false );
    void bind( imgHandle_t handle );
    SrcBlend::Modes getSrcBlendMode( const QString & );
    DstBlend::Modes getDstBlendMode( const QString & );
    void create2DVector( float x, float y, Renderer::Vec2D &array ) { array[0] = x; array[1] = y; }
    void create4DVector( float a, float b, float c, float d, Renderer::Vec4D &array  ) { array[0] = a; array[1] = b; array[2] = c; array[3] = d; }
    void setBlendMode( SrcBlend::Modes, DstBlend::Modes );

private:
    GLuint  texture;
    Renderer::Vec2D coords[Renderer::MaterialVertexes];
    Renderer::Vec2D texCoords[Renderer::MaterialVertexes];
    QColor  colour;
    QColor  savedColour;
    void    setCoords( float, float, float, float );
    void    setTextureCoords( float, float, float, float );
    void    setTextureCoords( const Renderer::Vec2D[Renderer::MaterialVertexes] );
    void    drawTriangleStrip();
    void    setAnimation( R_MaterialStage * );
    float   evaluateFunction( GenFunc *func );
    void    calcScrollTexCoords( R_MaterialStage * );
    void    calcScaleTexCoords( R_MaterialStage * );
    void    calcRotateTexCoords( R_MaterialStage * );
    void    calcTransformTexCoords( R_MaterialStage * );
    void    calcTurbulentTexCoords( R_MaterialStage * );
    void    calcStretchTexCoords( R_MaterialStage * );
    void    calcAlphaFunc( R_MaterialStage * );

signals:

public slots:
    void    saveColour() { this->savedColour = this->colour; }
    void    restoreColour() { this->setColour( this->savedColour ); }
};

//
// externals
//
#ifdef R_BUILD
extern class R_Cmd rCmd;
#endif

#endif // R_CMD_H
