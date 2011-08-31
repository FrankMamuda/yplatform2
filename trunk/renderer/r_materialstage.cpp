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

//
// includes
//
#include "r_image.h"
#include "r_material.h"
#include "r_materialstage.h"
#include "r_main.h"
#include "r_cmd.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

/*
===================
setDefaults
===================
*/
void R_MaterialStage::setDefaults( const QString &texture, const QString &clampMode ) {
    int y;

    if ( !texture.isEmpty())
        this->setTexture( texture );

    // set defaults
    this->setBlendMode( "blend" );
    this->setCoords( 0, 0, 1, 1 );
    this->setClampMode( clampMode );
    for ( y = 0; y < Renderer::NumStageImages; y++ )
        this->imageList[y] = -1;

    // clear data
    this->m_alphaGen = new AlphaGen();
    this->m_textureMod = new TextureMod();
    this->m_colourGen = new ColourGen();
    this->textureMod()->setType( TextureMod::None );
    this->alphaGen()->setType( AlphaGen::None );
    this->colourGen()->setType( ColourGen::None );

    // add to stageList (for garbage cleanup)
    m.mtrStageList << this;
}

/*
===================
construct
===================
*/
R_MaterialStage::R_MaterialStage( const QString &texture, const QString &clampMode, QObject *parent ) : QObject( parent ) {
    // init
    this->setDefaults( texture, clampMode );
}

/*
===================
destruct
===================
*/
R_MaterialStage::~R_MaterialStage() {
}

/*
===================
construct
===================
*/
R_MaterialStage::R_MaterialStage( const QString &texture, R_Material *mtrPtr, const QString &clampMode ) {
    if ( mtrPtr == NULL ) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MaterialStage::construct: NULL parent material\n" ));
        return;
    }

    // init
    this->setDefaults( texture, clampMode );

    // add to parent material
    mtrPtr->addStage( this );
    this->setParent( mtrPtr );
}

/*
===================
setBlendMode
===================
*/
void R_MaterialStage::setBlendMode( const QString &src, const QString &dst ) {
    this->m_srcBlend = ( int )cmd.getSrcBlendMode( src );
    this->m_dstBlend = ( int )cmd.getDstBlendMode( dst );
}

/*
===================
setBlendMode
===================
*/
void R_MaterialStage::setBlendMode( const QString &blendMode ) {
    if ( blendMode.isEmpty()) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MaterialStage::setBlendMode: no blend mode specified\n" ));
        return;
    }

    if ( !QString::compare( blendMode, "add", Qt::CaseInsensitive ))
        this->setBlendMode( "one", "one" );
    else if ( !QString::compare( blendMode, "filter", Qt::CaseInsensitive ))
        this->setBlendMode( "colour", "zero" );
    else if ( !QString::compare( blendMode, "blend", Qt::CaseInsensitive ))
        this->setBlendMode( "srcAlpha", "minusSrcAlpha" );
    else {
        mt.comPrint( this->tr( "^3WARNING: R_Material::setBlendMode: unknown blend mode \'%1\', setting dst blend 'one'\n" ).arg( blendMode ));
        this->setBlendMode( "one", "one" );
    }
}

/*
===================
setAnimationExt
===================
*/
void R_MaterialStage::setAnimationExt( const QStringList &textures, float speed ) {
    this->m_animation.clear();
    this->m_animation << textures;
    this->setAnimationSpeed( speed );
}

/*
===================
setTexture
===================
*/
void R_MaterialStage::setTexture( const QString &texture, const QString &blendMode ) {
    this->setTexture( texture );
    this->setBlendMode( blendMode );
}

/*
===================
setTexture
===================
*/
void R_MaterialStage::setTexture( const QString &texture, const QString &src, const QString &dst ) {
    this->setTexture( texture );
    this->setSrcBlend(( int )cmd.getSrcBlendMode( src ));
    this->setDstBlend(( int )cmd.getDstBlendMode( dst ));
}

/*
===================
setTexture
===================
*/
void R_MaterialStage::setTexture( int texture, const QString &blendMode ) {
    this->imageList[0] = (imgHandle_t)texture;
    this->setBlendMode( blendMode );
}

/*
===================
setTexture
===================
*/
void R_MaterialStage::setTexture( int texture, const QString &src, const QString &dst ) {
    this->imageList[0] = (imgHandle_t)texture;
    this->setSrcBlend(( int )cmd.getSrcBlendMode( src ));
    this->setDstBlend(( int )cmd.getDstBlendMode( dst ));
}

/*
===================
setCoords
===================
*/
void R_MaterialStage::setCoords( float s1, float t1, float s2, float t2 ) {
    this->m_coords[S1] = s1;
    this->m_coords[T1] = t1;
    this->m_coords[S2] = s2;
    this->m_coords[T2] = t2;
}

/*
===================
scale
===================
*/
void R_MaterialStage::scale( float s, float t ) {
    // set data
    this->textureMod()->scale[0] = s;
    this->textureMod()->scale[1] = t;
    this->textureMod()->setType( TextureMod::Scale );
}

/*
===================
scroll
===================
*/
void R_MaterialStage::scroll( float s, float t ) {
    // set data
    this->textureMod()->scroll[0] = s;
    this->textureMod()->scroll[1] = t;
    this->textureMod()->setType( TextureMod::Scroll );
}

/*
===================
rotate
===================
*/
void R_MaterialStage::rotate( float speed ) {
    // set data
    this->textureMod()->rotateSpeed = speed;
    this->textureMod()->setType( TextureMod::Rotate );
}

/*
===================
transform
===================
*/
void R_MaterialStage::transform( float m0, float m1, float m2, float t0, float t1 ) {
    // set data
    this->textureMod()->matrix[0][1] = m0;
    this->textureMod()->matrix[1][0] = m1;
    this->textureMod()->matrix[1][1] = m2;
    this->textureMod()->translate[0] = t0;
    this->textureMod()->translate[1] = t1;
    this->textureMod()->setType( TextureMod::Transform );
}

/*
===================
turb
===================
*/
void R_MaterialStage::turb( float base, float amplitude, float phase, float frequency ) {
    // set data
    this->textureMod()->func()->setBase( base );
    this->textureMod()->func()->setAmplitude( amplitude );
    this->textureMod()->func()->setPhase( phase );
    this->textureMod()->func()->setFrequency( frequency );
    this->textureMod()->setType( TextureMod::Turbulent );
}

/*
===================
stretch
===================
*/
void R_MaterialStage::stretch( const QString &func, float base, float amplitude, float phase, float frequency ) {
    // set data
    this->textureMod()->func()->setBase( base );
    this->textureMod()->func()->setAmplitude( amplitude );
    this->textureMod()->func()->setPhase( phase );
    this->textureMod()->func()->setFrequency( frequency );
    this->textureMod()->setType( TextureMod::Stretch );
    this->textureMod()->func()->setType( this->textureMod()->func()->typeForName( func ));

    if ( this->textureMod()->func()->type() == GenFunc::None )
        this->textureMod()->setType( TextureMod::None );
}

/*
===================
setTextureMod
===================
*/
void R_MaterialStage::setTextureMod( const QString &name, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4 ) {
    if ( !QString::compare( name, "turb", Qt::CaseInsensitive ))
        this->turb( arg0.toFloat(), arg1.toFloat(), arg2.toFloat(), arg3.toFloat());
    else if ( !QString::compare( name, "stretch", Qt::CaseInsensitive ))
        this->stretch( arg0.toString(), arg1.toFloat(), arg2.toFloat(), arg3.toFloat(), arg4.toFloat());
    else if ( !QString::compare( name, "transform", Qt::CaseInsensitive ))
        this->transform( arg0.toFloat(), arg1.toFloat(), arg2.toFloat(), arg3.toFloat(), arg4.toFloat());
    else if ( !QString::compare( name, "rotate", Qt::CaseInsensitive ))
        this->rotate( arg0.toFloat());
    else if ( !QString::compare( name, "scroll", Qt::CaseInsensitive ))
        this->scroll( arg0.toFloat(), arg1.toFloat());
    else if ( !QString::compare( name, "scale", Qt::CaseInsensitive ))
        this->scale( arg0.toFloat(), arg1.toFloat());
    else {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MaterialStage::setTextureMod: unknown texture mod \"%1\"\n" ).arg( name ));
        this->textureMod()->setType( TextureMod::None );
    }
}

/*
===================
setAlphaFunc
===================
*/
void R_MaterialStage::setAlphaFunc( const QString &func, float base, float amplitude, float phase, float frequency ) {
    // set data    
    this->alphaGen()->func()->setBase( base );
    this->alphaGen()->func()->setAmplitude( amplitude );
    this->alphaGen()->func()->setPhase( phase );
    this->alphaGen()->func()->setFrequency( frequency );
    this->alphaGen()->setType( AlphaGen::Func );
    this->alphaGen()->func()->setType( this->alphaGen()->func()->typeForName( func ));

    if ( this->alphaGen()->func()->type() == GenFunc::None )
        this->alphaGen()->setType( AlphaGen::None );
}

/*
===================
setAlphaGen
===================
*/
void R_MaterialStage::setAlphaGen( const QString &name, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4 ) {
    if ( !QString::compare( name, "const", Qt::CaseInsensitive )) {
        this->alphaGen()->setType( AlphaGen::Const );
        this->alphaGen()->alpha = arg0.toFloat();
    } else if ( !QString::compare( name, "func", Qt::CaseInsensitive )) {
        this->setAlphaFunc( arg0.toString(), arg1.toFloat(), arg2.toFloat(), arg3.toFloat(), arg4.toFloat());
    } else {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MaterialStage::setAlphaGen: unknown alpha gen \"%1\"\n" ).arg( name ));
        this->alphaGen()->setType( AlphaGen::None );
    }
}

/*
===================
setColourGen
===================
*/
void R_MaterialStage::setColourGen( const QString &name, const QVariant &arg0, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4 ) {
    Q_UNUSED( arg3 )
    Q_UNUSED( arg4 )

    if ( !QString::compare( name, "const", Qt::CaseInsensitive )) {
        this->colourGen()->setType( ColourGen::Const );
        this->colourGen()->colour = QColor::fromRgbF( arg0.toFloat(), arg1.toFloat(), arg2.toFloat());
    } else {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MaterialStage::setColourGen: unknown colour gen \"%1\"\n" ).arg( name ));
        this->colourGen()->setType( ColourGen::None );
    }
}

/*
================
tableForType
================
*/
float *GenFunc::tableForType() {
    switch ( this->type()) {
    case Sine:
        return m.funcTable[GenFunc::Sine];

    case Square:
        return m.funcTable[GenFunc::Square];

    case Triangle:
        return m.funcTable[GenFunc::Triangle];

    case SawTooth:
        return m.funcTable[GenFunc::SawTooth];

    case InverseSawtooth:
        return m.funcTable[GenFunc::InverseSawtooth];

    case None:
    default:
        break;
    }

    // this should not happen
    mt.comError( Sys_Common::SoftError, this->tr( "GenFunc::tableForType: invalid function type '%1'\n" ).arg( this->type()));
    return NULL;
}

/*
===============
getGenFunc
===============
*/
GenFunc::Types GenFunc::typeForName( const QString &name ) const {
    if ( !QString::compare( name, "sin", Qt::CaseInsensitive ) || !QString::compare( name, "sine", Qt::CaseInsensitive ))
        return Sine;
    else if ( !QString::compare( name, "square", Qt::CaseInsensitive ))
        return Square;
    else if ( !QString::compare( name, "triangle", Qt::CaseInsensitive ))
        return Triangle;
    else if ( !QString::compare( name, "sawTooth", Qt::CaseInsensitive ))
        return SawTooth;
    else if ( !QString::compare( name, "inverseSawTooth", Qt::CaseInsensitive ))
        return InverseSawtooth;
    else if ( !QString::compare( name, "none", Qt::CaseInsensitive ))
        return None;

    // this should not happen
    mt.comError( Sys_Common::SoftError, this->tr( "GenFunc::typeForName: invalid genFunc '%1'\n" ).arg( name ));
    return None;
}

/*
===================
clampMode
===================
*/
void R_MaterialStage::setClampMode( const QString &mode ) {
    if ( !QString::compare( mode, "repeat", Qt::CaseInsensitive )) {
        this->setClampMode( R_Image::Repeat );
    } else if ( !QString::compare( mode, "clamp", Qt::CaseInsensitive )) {
        this->setClampMode( R_Image::Clamp );
    } else if ( !QString::compare( mode, "clampToEdge", Qt::CaseInsensitive ) || !QString::compare( mode, "edge", Qt::CaseInsensitive )) {
        this->setClampMode( R_Image::ClampToEdge );
    } else {
        mt.comError( Sys_Common::SoftError, this->tr( "R_MaterialStage::setClampMode: unknown clamp mode \"%1\"\n" ).arg( this->m_clamp ));
        this->setClampMode( R_Image::Repeat );
    }
}

