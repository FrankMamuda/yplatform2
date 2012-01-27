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

#ifndef R_MATERIALSTAGE_H
#define R_MATERIALSTAGE_H

//
// includes
//
#include "r_shared.h"
#include "r_material.h"
#include "r_image.h"
#include <QtScript>

//
// namespaces
//
namespace Renderer {
    static const int NumStageImages = 32;
}

//
// classes
//
class R_Material;

//
// class:GenFunc
//
class GenFunc : public QObject {
    Q_OBJECT
    Q_ENUMS( Types )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( float base READ base WRITE setBase )
    Q_PROPERTY( float amplitude READ amplitude WRITE setAmplitude )
    Q_PROPERTY( float phase READ phase WRITE setPhase )
    Q_PROPERTY( float frequency READ frequency WRITE setFrequency )

public:
    enum Types {
        None = 0,
        Sine,
        Square,
        Triangle,
        SawTooth,
        InverseSawtooth
    };
    explicit GenFunc() {}
    ~GenFunc() {}
    Types type() const { return this->m_type; }
    float base() const { return this->m_base; }
    float amplitude() const { return this->m_amplitude; }
    float phase() const { return this->m_phase; }
    float frequency() const { return this->m_frequency; }
    float *tableForType();
    Types typeForName( const QString & ) const;

public slots:
    void setType( Types type ) { this->m_type = type; }
    void setBase( float base ) { this->m_base = base; }
    void setAmplitude( float amplitude ) { this->m_amplitude = amplitude; }
    void setPhase( float phase ) { this->m_phase = phase; }
    void setFrequency( float frequency ) { this->m_frequency = frequency; }

private:
    Types m_type;
    float m_base;
    float m_amplitude;
    float m_phase;
    float m_frequency;
};

//
// class:TextureMod
//
class TextureMod : public QObject {
    Q_OBJECT
    Q_ENUMS( TextureMods )
    Q_PROPERTY( TextureMods type READ type WRITE setType )

public:
    enum TextureMods {
        None = 0,
        Transform,
        Turbulent,
        Scroll,
        Scale,
        Stretch,
        Rotate
    };
    explicit TextureMod() { this->m_func = new GenFunc(); }
    ~TextureMod() { delete this->m_func; }
    TextureMods type() const { return this->m_type; }
    GenFunc     *func() { return this->m_func; }
    float       matrix[2][2];
    float       translate[2];
    float       scale[2];
    float       scroll[2];
    float       rotateSpeed;

public slots:
    void setType( TextureMods type ) { this->m_type = type; }

private:
    TextureMods m_type;
    GenFunc     *m_func;
};

//
// class:AlphaGen
//
class AlphaGen : public QObject {
    Q_OBJECT
    Q_ENUMS( AlphaGens )
    Q_PROPERTY( AlphaGens type READ type WRITE setType )

public:
    enum AlphaGens {
        None = 0,
        Const,
        Func
    };
    explicit AlphaGen() { this->m_func = new GenFunc(); }
    ~AlphaGen() { delete this->m_func; }
    AlphaGens type() const { return this->m_type; }
    GenFunc   *func() { return this->m_func; }
    float alpha;

public slots:
    void setType( AlphaGens type ) { this->m_type = type; }

private:
    AlphaGens m_type;
    GenFunc   *m_func;
};

//
// class:ColourGen
//
class ColourGen : public QObject {
    Q_OBJECT
    Q_ENUMS( ColourGens )
    Q_PROPERTY( ColourGens type READ type WRITE setType )

public:    
    enum ColourGens {
        None = 0,
        Const
    };
    ColourGens type() const { return this->m_type; }
    QColor colour;

public slots:
    void setType( ColourGens type ) { this->m_type = type; }

private:
    ColourGens m_type;
};

//
// class:R_MaterialStage
//
class R_MaterialStage : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Material stage" )
    Q_DISABLE_COPY( R_MaterialStage )
    Q_ENUMS( Coords )

public:
    enum Coords {
        S1 = 0,
        T1,
        S2,
        T2
    };
    explicit R_MaterialStage( const QString &texture = QString(), const QString &clampMode = QString( "repeat" ), QObject *parent = 0 );
    explicit R_MaterialStage( const QString &texture, R_Material *mtrPtr, const QString &clampMode = QString( "repeat" ));
    ~R_MaterialStage();
    imgHandle_t imageList[Renderer::NumStageImages];
    QStringList *animation() { return &this->m_animation; }
    TextureMod *textureMod() { return this->m_textureMod; }
    AlphaGen *alphaGen() { return this->m_alphaGen; }
    ColourGen *colourGen() { return this->m_colourGen; }
    unsigned int srcBlend() const { return this->m_srcBlend; }
    unsigned int dstBlend() const { return this->m_dstBlend; }
    float animationSpeed() const { return this->m_animationSpeed; }
    QString texture() const { return m_texture; }
    float coords( Coords texCoords ) const { return this->m_coords[texCoords]; }
    R_Image::ClampModes clampMode() const { return this->m_clamp; }

private:
    TextureMod *m_textureMod;
    AlphaGen *m_alphaGen;
    ColourGen *m_colourGen;
    unsigned int m_srcBlend;
    unsigned int m_dstBlend;
    float m_animationSpeed;
    QString m_texture;
    Renderer::Vec4D m_coords;
    R_Image::ClampModes m_clamp;
    QStringList m_animation;

private slots:
    void setDefaults( const QString &texture, const QString & );
    void setAnimationExt( const QStringList &textures, float speed );

    // texMods
    void scale( float s, float t );
    void scroll( float s, float t );
    void rotate( float speed );
    void transform( float m0, float m1, float m2, float t0, float t1 );
    void turb( float base, float amplitude, float phase, float frequency );
    void stretch( const QString &func, float base, float amplitude, float phase, float frequency );

    // clamp modes
    void setClampMode( R_Image::ClampModes mode ) { this->m_clamp = mode; }
    void setClampMode( const QString &mode );

public slots:
    void setBlendMode( const QString &, const QString & );
    void setBlendMode( const QString & = QString( "blend" ));
    void setTexture( const QString &texture ) { this->m_texture = texture; }
    void setTexture( const QString &, const QString & );
    void setTexture( const QString &, const QString &, const QString & );
    void setTexture( int texture ) { this->imageList[0] = static_cast<imgHandle_t>( texture ); }
    void setTexture( int texture, const QString & );
    void setTexture( int texture, const QString &, const QString & );
    void setCoords( float s1 = 0.0f, float t1  = 0.0f, float s2  = 1.0f, float t2 = 1.0f );
    void setTextureMod( const QString &name, const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant());
    void setAnimation( const QString &tx0, const QString &tx1, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, const QString &tx7, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, const QString &tx7, const QString &tx8, float speed );
    void setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, const QString &tx7, const QString &tx8, const QString &tx9, float speed );
    void setAlphaGen( const QString &name, const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant());
    void setAlphaFunc( const QString &func, float base, float amplitude, float phase, float frequency );
    void setColourGen( const QString &name, const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant(), const QVariant & = QVariant());
    void setAnimationSpeed( float speed ) { this->m_animationSpeed = speed; }
    void setSrcBlend ( unsigned int srcBlend ) { this->m_srcBlend = srcBlend; }
    void setDstBlend ( unsigned int dstBlend ) { this->m_dstBlend = dstBlend; }
    void addFrame( const QString &texture ) { this->m_animation << texture; }
};

//
// inlines for R_MaterialStage::setAnimation
//
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, float speed ) { QStringList textureList; textureList << tx0 << tx1; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3 << tx4; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3 << tx4 << tx5; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3 << tx4 << tx5 << tx6; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, const QString &tx7, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3 << tx4 << tx5 << tx6 << tx7; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, const QString &tx7, const QString &tx8, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3 << tx4 << tx5 << tx6 << tx7 << tx8; this->setAnimationExt( textureList, speed ); }
inline void R_MaterialStage::setAnimation( const QString &tx0, const QString &tx1, const QString &tx2, const QString &tx3, const QString &tx4, const QString &tx5, const QString &tx6, const QString &tx7, const QString &tx8, const QString &tx9, float speed ) { QStringList textureList; textureList << tx0 << tx1 << tx2 << tx3 << tx4 << tx5 << tx6 << tx7 << tx8 << tx9; this->setAnimationExt( textureList, speed ); }

#endif // R_MATERIALSTAGE_H
