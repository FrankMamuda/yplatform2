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

//
// includes
//
#include "r_cmd.h"
#include "r_main.h"
#include "r_glimp.h"
#include "r_material.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

//
// classes
//
class R_Cmd rCmd;

/*
================
bind
================
*/
void R_Cmd::bind( imgHandle_t handle ) {
    const R_Image *image;

    // failsafe
    if ( handle < 0 || handle >= m.imageList.count())
        handle = m.defaultImage;

    // get image
    image = m.imageList.at( handle );

    // this really should not happen, but just in case
    if ( !image ) {
        com.error( StrFatalError + this->tr( "called with NULL image\n" ));
        return;
    }

    // bind texture
    if ( this->texture != image->texture ) {
        this->texture = image->texture;

        // bind texture
        glBindTexture( GL_TEXTURE_2D, image->texture );
    }
}

/*
===============
setCoords
===============
*/
void R_Cmd::setCoords( float x, float y, float w, float h ) {
    glImp.adjustCoords( x, y );
    glImp.adjustCoords( w, h );

    // add vert coords
    if ( w > 0 && h > 0 ) {
        this->create2DVector( x, y, this->coords[0] );
        this->create2DVector( x + w, y, this->coords[1] );
        this->create2DVector( x + w, y + h, this->coords[2] );
        this->create2DVector( x, y + h, this->coords[3] );
    } else if ( w < 0 && h > 0 ) {
        this->create2DVector( x + -w, y, this->coords[0] );
        this->create2DVector( x, y, this->coords[1] );
        this->create2DVector( x, y + h, this->coords[2] );
        this->create2DVector( x + -w, y + h, this->coords[3] );
    } else if ( w > 0 && h < 0 ) {
        this->create2DVector( x, y + -h, this->coords[0] );
        this->create2DVector( x + w, y + -h, this->coords[1] );
        this->create2DVector( x + w, y, this->coords[2] );
        this->create2DVector( x, y, this->coords[3] );
    } else if ( w > 0 && h > 0 ) {
        this->create2DVector( x + -w, y + -h, this->coords[0] );
        this->create2DVector( x, y + -h, this->coords[1] );
        this->create2DVector( x, y, this->coords[2] );
        this->create2DVector( x + -w, y, this->coords[3] );
    }
}

/*
===============
setTextureCoords
===============
*/
void R_Cmd::setTextureCoords( float s1, float t1, float s2, float t2 ) {
    // add texture coords
    this->create2DVector( s1, t1, this->texCoords[0] );
    this->create2DVector( s2, t1, this->texCoords[1] );
    this->create2DVector( s2, t2, this->texCoords[2] );
    this->create2DVector( s1, t2, this->texCoords[3] );
}

/*
===============
setTextureCoords
===============
*/
void R_Cmd::setTextureCoords( const Renderer::Vec2D st[4] ) {
    int y;

    // add new set of coords
    for ( y = 0; y < Renderer::MaterialVertexes; y++ )
        this->create2DVector( st[y][0], st[y][1], this->texCoords[y] );
}

/*
===============
drawTriangleStrip
===============
*/
void R_Cmd::drawTriangleStrip() {
    int y;

    // begin drawing triangle strip
    glBegin( GL_TRIANGLE_STRIP );
    for ( y = 0; y < Renderer::TriSurfVertexes; y++ ) {
        glColor4f( this->colour.redF(), this->colour.greenF(), this->colour.blueF(), this->colour.alphaF());
        glTexCoord2f( this->texCoords[Renderer::TriSurf[y]][0], this->texCoords[Renderer::TriSurf[y]][1]);
        glVertex2f( this->coords[Renderer::TriSurf[y]][0], this->coords[Renderer::TriSurf[y]][1]);
    }
    glEnd();
}

/*
===============
getSrcBlendMode
===============
*/
R_Cmd::SrcBlend::Modes R_Cmd::getSrcBlendMode( const QString &name ) {
    if ( !QString::compare( name, "one", Qt::CaseInsensitive ))
        return SrcBlend::One;
    else if ( !QString::compare( name, "zero", Qt::CaseInsensitive ))
        return SrcBlend::Zero;
    else if ( !QString::compare( name, "colour", Qt::CaseInsensitive ) || !QString::compare( name, "color", Qt::CaseInsensitive ))
        return SrcBlend::Colour;
    else if ( !QString::compare( name, "minusColour", Qt::CaseInsensitive ) || !QString::compare( name, "minusColor", Qt::CaseInsensitive ) ||
              !QString::compare( name, "oneMinusColour", Qt::CaseInsensitive ) || !QString::compare( name, "oneMinusColor", Qt::CaseInsensitive ))
        return SrcBlend::MinusColour;
    else if ( !QString::compare( name, "srcAlpha", Qt::CaseInsensitive ))
        return SrcBlend::SrcAlpha;
    else if ( !QString::compare( name, "minusSrcAlpha", Qt::CaseInsensitive ) || !QString::compare( name, "oneMinusSrcAlpha", Qt::CaseInsensitive ))
        return SrcBlend::MinusSrcAlpha;
    else if ( !QString::compare( name, "dstAlpha", Qt::CaseInsensitive ))
        return SrcBlend::DstAlpha;
    else if ( !QString::compare( name, "minusDstAlpha", Qt::CaseInsensitive ) || !QString::compare( name, "oneMinusDstAlpha", Qt::CaseInsensitive ))
        return SrcBlend::MinusDstAlpha;
    else if ( !QString::compare( name, "saturate", Qt::CaseInsensitive ) || !QString::compare( name, "alphaSaturate", Qt::CaseInsensitive ))
        return SrcBlend::AlphaSaturate;

    // this should not happen
    com.error( StrSoftError + this->tr( "unknown blend mode \'%1\', setting src blend 'one'\n" ).arg( name ));
    return SrcBlend::One;
}

/*
===============
getDstBlendMode
===============
*/
R_Cmd::DstBlend::Modes R_Cmd::getDstBlendMode( const QString &name ) {
    if ( !QString::compare( name, "one", Qt::CaseInsensitive ))
        return DstBlend::One;
    else if ( !QString::compare( name, "zero", Qt::CaseInsensitive ))
        return DstBlend::Zero;
    else if ( !QString::compare( name, "srcAlpha", Qt::CaseInsensitive ))
        return DstBlend::SrcAlpha;
    else if ( !QString::compare( name, "minusSrcAlpha", Qt::CaseInsensitive ) || !QString::compare( name, "oneMinusSrcAlpha", Qt::CaseInsensitive ))
        return DstBlend::MinusSrcAlpha;
    else if ( !QString::compare( name, "dstAlpha", Qt::CaseInsensitive ))
        return DstBlend::DstAlpha;
    else if ( !QString::compare( name, "minusDstAlpha", Qt::CaseInsensitive ) || !QString::compare( name, "oneMinusDstAlpha", Qt::CaseInsensitive ))
        return DstBlend::MinusDstAlpha;
    else if ( !QString::compare( name, "colour", Qt::CaseInsensitive ) || !QString::compare( name, "color", Qt::CaseInsensitive ))
        return DstBlend::Colour;
    else if ( !QString::compare( name, "minusColour", Qt::CaseInsensitive ) || !QString::compare( name, "minusColor", Qt::CaseInsensitive ) ||
              !QString::compare( name, "oneMinusColour", Qt::CaseInsensitive ) || !QString::compare( name, "oneMinusColor", Qt::CaseInsensitive ))
        return DstBlend::MinusColour;

    // this should not happen
    com.error( StrSoftError + this->tr( "unknown blend mode \'%1\', setting dst blend 'one'\n" ).arg( name ));
    return DstBlend::One;
}

/*
================
setBlendMode
================
*/
void R_Cmd::setBlendMode( SrcBlend::Modes srcBlend, DstBlend::Modes dstBlend ) {
    GLenum srcFactor;
    GLenum dstFactor;

    if ( !srcBlend || !dstBlend )
        return;

    switch ( srcBlend ) {
    case SrcBlend::Zero:
        srcFactor = GL_ZERO;
        break;

    case SrcBlend::One:
        srcFactor = GL_ONE;
        break;

    case SrcBlend::Colour:
        srcFactor = GL_DST_COLOR;
        break;

    case SrcBlend::MinusColour:
        srcFactor = GL_ONE_MINUS_DST_COLOR;
        break;

    case SrcBlend::SrcAlpha:
        srcFactor = GL_SRC_ALPHA;
        break;

    case SrcBlend::MinusSrcAlpha:
        srcFactor = GL_ONE_MINUS_SRC_ALPHA;
        break;

    case SrcBlend::DstAlpha:
        srcFactor = GL_DST_ALPHA;
        break;

    case SrcBlend::MinusDstAlpha:
        srcFactor = GL_ONE_MINUS_DST_ALPHA;
        break;

    case SrcBlend::AlphaSaturate:
        srcFactor = GL_SRC_ALPHA_SATURATE;
        break;

    default:
        srcFactor = SrcBlend::One;
        com.error( StrSoftError + this->tr( "invalid source blend mode\n" ));
        break;
    }

    switch ( dstBlend ) {
    case DstBlend::Zero:
        dstFactor = GL_ZERO;
        break;

    case DstBlend::One:
        dstFactor = GL_ONE;
        break;

    case DstBlend::Colour:
        dstFactor = GL_SRC_COLOR;
        break;

    case DstBlend::MinusColour:
        dstFactor = GL_ONE_MINUS_SRC_COLOR;
        break;

    case DstBlend::SrcAlpha:
        dstFactor = GL_SRC_ALPHA;
        break;

    case DstBlend::MinusSrcAlpha:
        dstFactor = GL_ONE_MINUS_SRC_ALPHA;
        break;

    case DstBlend::DstAlpha:
        dstFactor = GL_DST_ALPHA;
        break;

    case DstBlend::MinusDstAlpha:
        dstFactor = GL_ONE_MINUS_DST_ALPHA;
        break;

    default:
        dstFactor = DstBlend::One;
        com.error( StrSoftError + this->tr( "invalid destination blend mode\n" ));
        break;
    }

    glEnable( GL_BLEND );
    glBlendFunc( srcFactor, dstFactor );
}

/*
===============
drawImage
===============
*/
void R_Cmd::drawImage( float x, float y, float w, float h, float s1, float t1, float s2, float t2, imgHandle_t handle ) {
    // set rendering mode to 2D
    if ( !glImp.widget->projection2D())
        glImp.widget->setProjection2D();

    // setup coords
    this->setCoords( x, y, w, h );
    this->setTextureCoords( s1, t1, s2, t2 );

    // enable texturing
    glEnable( GL_TEXTURE_2D );

    // enable blending
    this->setBlendMode( SrcBlend::SrcAlpha, DstBlend::MinusSrcAlpha );

    // bind texture
    this->bind( handle );

    // draw triangle
    this->drawTriangleStrip();

    // disable texturing
    glDisable( GL_TEXTURE_2D );
}

/*
================
calcScrollTexCoords
================
*/
void R_Cmd::calcScrollTexCoords( R_MaterialStage *stagePtr ) {
    float aScrollS;
    float aScrollT;

    // calc offset
    aScrollS = stagePtr->textureMod()->scroll[0] * m.time();
    aScrollT = stagePtr->textureMod()->scroll[1] * m.time();

    // clamp
    aScrollS = aScrollS - floor( aScrollS );
    aScrollT = aScrollT - floor( aScrollT );

    // set coords
    this->setTextureCoords( stagePtr->coords( R_MaterialStage::S1 ) + aScrollS, stagePtr->coords( R_MaterialStage::T1 ) + aScrollT, stagePtr->coords( R_MaterialStage::S2 ) + aScrollS, stagePtr->coords( R_MaterialStage::T2 ) + aScrollT );
}

/*
================
calcScaleTexCoords
================
*/
void R_Cmd::calcScaleTexCoords( R_MaterialStage *stagePtr ) {
    // setup texture coords
    this->setTextureCoords( stagePtr->coords( R_MaterialStage::S1 ) * stagePtr->textureMod()->scale[0], stagePtr->coords( R_MaterialStage::T1 ) * stagePtr->textureMod()->scale[1], stagePtr->coords( R_MaterialStage::S2 ) * stagePtr->textureMod()->scale[0], stagePtr->coords( R_MaterialStage::T2 ) * stagePtr->textureMod()->scale[1] );
}

/*
================
calcRotateTexCoords
================
*/
void R_Cmd::calcRotateTexCoords( R_MaterialStage *stagePtr ) {
    float degrees;
    int index;
    float sinValue, cosValue;

    // perform calclations
    degrees = -stagePtr->textureMod()->rotateSpeed * m.time();
    index = static_cast<int>( degrees * ( Renderer::FuncTableSize / 360.0f ));
    sinValue = m.funcTable[GenFunc::Sine][index & ( Renderer::FuncTableSize - 1 )];
    cosValue = m.funcTable[GenFunc::Sine][( index + Renderer::FuncTableSize / 4 ) & ( Renderer::FuncTableSize - 1 )];

    // set as transformation matrix
    stagePtr->textureMod()->matrix[0][0] = cosValue;
    stagePtr->textureMod()->matrix[1][0] = -sinValue;
    stagePtr->textureMod()->translate[0] = 0.5f - 0.5f * cosValue + 0.5f * sinValue;
    stagePtr->textureMod()->matrix[0][1] = sinValue;
    stagePtr->textureMod()->matrix[1][1] = cosValue;
    stagePtr->textureMod()->translate[1] = 0.5f - 0.5f * sinValue - 0.5f * cosValue;

    // execute transformation
    this->calcTransformTexCoords( stagePtr );
}

/*
================
calcTransformTexCoords
================
*/
void R_Cmd::calcTransformTexCoords( R_MaterialStage *stagePtr ) {
    int y;
    float s, t;
    Renderer::Vec2D transformCoords[Renderer::MaterialVertexes];

    // calc each vertex
    for ( y = 0; y < Renderer::MaterialVertexes; y++ ) {
        s = this->texCoords[y][0];
        t = this->texCoords[y][1];

        this->create2DVector( s * stagePtr->textureMod()->matrix[0][0] + t * stagePtr->textureMod()->matrix[1][0] + stagePtr->textureMod()->translate[0],
                              s * stagePtr->textureMod()->matrix[0][1] + t * stagePtr->textureMod()->matrix[1][1] + stagePtr->textureMod()->translate[1],
                              transformCoords[y] );
    }

    // set coords
    this->setTextureCoords( transformCoords );
}

/*
================
calcTurbulentTexCoords
================
*/
void R_Cmd::calcTurbulentTexCoords( R_MaterialStage *stagePtr ) {
    int y;
    float value;
    float s, t;
    Renderer::Vec2D turbCoords[Renderer::MaterialVertexes];

    // calc current function value
    value = ( stagePtr->textureMod()->func()->phase() + m.time() * stagePtr->textureMod()->func()->frequency());

    // calc each vertex
    for ( y = 0; y < Renderer::MaterialVertexes; y++ ) {
        s = this->texCoords[y][0];
        t = this->texCoords[y][1];

        this->create2DVector( s + m.funcTable[GenFunc::Sine][( static_cast<int>((( this->coords[y][0]) * 1.0f / 128 * 0.125f + value ) * Renderer::FuncTableSize )) & ( Renderer::FuncTableSize - 1 )] * stagePtr->textureMod()->func()->amplitude(),
                              t + m.funcTable[GenFunc::Sine][( static_cast<int>(( this->coords[y][1] * 1.0f / 128 * 0.125f + value ) * Renderer::FuncTableSize )) & ( Renderer::FuncTableSize - 1 )] * stagePtr->textureMod()->func()->amplitude(),
                              turbCoords[y] );
    }

    // set coords
    this->setTextureCoords( turbCoords );
}

/*
================
evaluateFunction
================
*/
float R_Cmd::evaluateFunction( GenFunc *func ) {
    // evaluate function
    return func->base() + func->tableForType()[myftol(( func->phase() + m.time() * func->frequency()) * Renderer::FuncTableSize ) & ( Renderer::FuncTableSize - 1 )] * func->amplitude();
}

/*
================
calcStretchTexCoords
================
*/
void R_Cmd::calcStretchTexCoords( R_MaterialStage *stagePtr ) {
    float value;

    // calc current function value
    value = 1.0f / this->evaluateFunction( stagePtr->textureMod()->func());

    // set as transformation matrix
    stagePtr->textureMod()->matrix[0][0] = value;
    stagePtr->textureMod()->matrix[1][0] = 0.0f;
    stagePtr->textureMod()->translate[0] = 0.5f - 0.5f * value;
    stagePtr->textureMod()->matrix[0][1] = 0.0f;
    stagePtr->textureMod()->matrix[1][1] = value;
    stagePtr->textureMod()->translate[1] = 0.5f - 0.5f * value;

    // set coords
    this->calcTransformTexCoords( stagePtr );
}

/*
================
setAnimation
================
*/
void R_Cmd::setAnimation( R_MaterialStage *stagePtr ) {
    int y;

    // calc
    y = myftol( m.time() * stagePtr->animationSpeed() * Renderer::FuncTableSize );
    y >>= 10;

    // may happen with material time offsets
    if ( y < 0 )
        y = 0;

    y %= stagePtr->animation()->count();

    // find/register & bind texture
    if ( stagePtr->imageList[y] == -1 )
        stagePtr->imageList[y] = m.loadImage( stagePtr->animation()->at( y ), stagePtr->clampMode());
    this->bind( stagePtr->imageList[y] );
}

/*
===============
calcAlphaFunc
===============
*/
void R_Cmd::calcAlphaFunc( R_MaterialStage *stagePtr ) {
    float value;

    // get value
    value = this->evaluateFunction( stagePtr->alphaGen()->func());

    // clamp value
    if ( value < 0 )
        value = 0.0f;
    else if ( value > 1 )
        value = 1.0f;

    // set vertex colour
    this->colour.setAlphaF( value );
}

/*
===============
drawMaterial
===============
*/
void R_Cmd::drawMaterial( float x, float y, float w, float h, mtrHandle_t handle ) {
    const R_Material *mtrPtr;

    // set rendering mode to 2D
    if ( !glImp.widget->projection2D())
        glImp.widget->setProjection2D();

    // failsafe
    if ( handle < 0 || handle >= m.mtrList.count())
        return;

    // save previous colour
    this->saveColour();

    // get material
    mtrPtr = m.mtrList.at( handle );

    // no stages at all?
    // no problem, search for an image
    if ( mtrPtr->stageList.isEmpty()) {
        // draw default image and return
        this->drawImage( x, y, w, h, m.loadImage( mtrPtr->name()));
        return;
    }

    // setup coords
    this->setCoords( x, y, w, h );

    // enable texturing
    glEnable( GL_TEXTURE_2D );

    // draw stages
    foreach ( R_MaterialStage *stagePtr, mtrPtr->stageList ) {
        this->setTextureCoords( stagePtr->coords( R_MaterialStage::S1 ),
                                stagePtr->coords( R_MaterialStage::T1 ),
                                stagePtr->coords( R_MaterialStage::S2 ),
                                stagePtr->coords( R_MaterialStage::T2 ));

        // execute textureMods
        switch ( stagePtr->textureMod()->type()) {
        case TextureMod::None:
            break;

        case TextureMod::Scale:
            // setup texture coords
            this->calcScaleTexCoords( stagePtr );
            break;

        case TextureMod::Scroll:
            // setup texture coords
            this->calcScrollTexCoords( stagePtr );
            break;

        case TextureMod::Rotate:
            // setup texture coords
            this->calcRotateTexCoords( stagePtr );
            break;

        case TextureMod::Transform:
            // setup texture coords
            this->calcTransformTexCoords( stagePtr );
            break;

        case TextureMod::Turbulent:
            // setup texture coords
            this->calcTurbulentTexCoords( stagePtr );
            break;

        case TextureMod::Stretch:
            // setup texture coords
            this->calcStretchTexCoords( stagePtr );
            break;

        default:
            com.error( StrFatalError + this->tr( "invalid textureMod '%1'\n" ).arg( stagePtr->textureMod()->type()));
            return;
        }

        // execute alphaGen
        switch ( stagePtr->alphaGen()->type()) {
        case AlphaGen::None:
            break;

        case AlphaGen::Const:
            this->colour.setAlphaF( stagePtr->alphaGen()->alpha );
            break;

        case AlphaGen::Func:
            this->calcAlphaFunc( stagePtr );
            break;

        default:
            com.error( StrFatalError + this->tr( "invalid alphaGen '%1'\n" ).arg( stagePtr->alphaGen()->type()));
            return;
        }

        // execute colourGen
        switch ( stagePtr->colourGen()->type()) {
        case ColourGen::None:
            break;

        case ColourGen::Const:
            this->colour.setRedF( stagePtr->colourGen()->colour.redF());
            this->colour.setGreenF( stagePtr->colourGen()->colour.greenF());
            this->colour.setBlueF( stagePtr->colourGen()->colour.blueF());
            break;

        default:
            com.error( StrFatalError + this->tr( "invalid colourGen '%1'\n" ).arg( stagePtr->colourGen()->type()));
            return;
        }

        // enable blending
        this->setBlendMode( static_cast<R_Cmd::SrcBlend::Modes>( stagePtr->srcBlend()), static_cast<R_Cmd::DstBlend::Modes>( stagePtr->dstBlend()));

        // handle animations
        if ( stagePtr->animation()->count() > 1 ) {
            this->setAnimation( stagePtr );
        } else {
            // find/register & bind texture
            if ( stagePtr->imageList[0] == -1 ) {
                // no texture set
                if ( !stagePtr->texture().isEmpty())
                    stagePtr->imageList[0] = m.loadImage( stagePtr->texture(), stagePtr->clampMode());
            }
            this->bind( stagePtr->imageList[0] );
        }

        // draw triangle
        this->drawTriangleStrip();
    }

    // disable texturing
    glDisable( GL_TEXTURE_2D );

    // restore colour
    this->restoreColour();
}

/*
===============
setColour
===============
*/
void R_Cmd::setColour( float r, float g, float b, float a, bool directly ) {
    // add new colours
    this->colour = QColor::fromRgbF( r, g, b, a );

    // used for font rendering
    if ( directly )
        glColor4f( r, g, b, a );
}

/*
===============
setColour
===============
*/
void R_Cmd::setColour( const QColor colour, bool directly ) {
    // add new colours
    this->colour = colour;

    // used for font rendering
    if ( directly )
        glColor4f( colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF());
}

/*
===============
setColour
===============
*/
void R_Cmd::setColour( const Renderer::Vec4D colour, bool directly ) {
    // add new colours
    this->colour = QColor::fromRgbF( colour[0], colour[1], colour[2], colour[3] );

    // used for font rendering
    if ( directly )
        glColor4f( colour[0], colour[1], colour[2], colour[3] );
}
