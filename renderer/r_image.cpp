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
#include "r_image.h"
#include "r_main.h"
#include "r_glimp.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

/*
===================
construct
===================
*/
R_Image::R_Image( const QString &filename, ClampModes mode, QObject *parent ) : QObject( parent ) {
    this->reload( filename, mode );
}

/*
===================
destruct
===================
*/
R_Image::~R_Image() {
    glDeleteTextures( 1, &this->texture );
}

/*
===================
reload
===================
*/
void R_Image::reload( const QString &filename, ClampModes mode ) {
    QImage image;
    QByteArray buffer;
    byte *imageBuffer = NULL;
#ifdef BUILTIN_TARGA_LOADER
    bool targa = false;
#endif

    // defaults
    this->validate( false );

    // set clamp mode
    this->setClampMode( mode );

    // load image
    buffer = fs.readFile( filename, Sys_Filesystem::Silent );
    if ( !buffer.isEmpty()) {
#ifdef BUILTIN_TARGA_LOADER
        if ( filename.endsWith( ".tga" )) {
            // win32 can't handle targas natively
            unsigned int w, h;
            imageBuffer = this->loadTargaImage( filename, buffer.length(), reinterpret_cast<byte*>( buffer.data()), w, h );
            this->setWidth( w );
            this->setHeight( h );
            targa = true;
        } else {
#endif
            image = QGLWidget::convertToGLFormat( QImage::fromData( buffer ).mirrored( false, true ));
            imageBuffer = reinterpret_cast<byte*>( image.bits());
            this->setWidth( image.width());
            this->setHeight( image.height());
#ifdef BUILTIN_TARGA_LOADER
        }
#endif

        // free buffer
        buffer.clear();
    }

    // abort if void
    if ( imageBuffer == NULL || this->width() == 0 || this->height() == 0 )
        return;

    // create texture
    this->createTexture( imageBuffer, this->width(), this->height());

#ifdef BUILTIN_TARGA_LOADER
    // must clear targa rgba buffer
    if ( targa )
        delete[] imageBuffer;
#endif

    // set filename (without extension)
    this->setName( QString( QFileInfo( filename ).path() + "/" + QFileInfo( filename ).baseName()));

    // all ok
    this->validate();
}

/*
===================
mipMap
===================
*/
void R_Image::mipMap( byte *inPtr, int w, int h ) {
    int		y, k;
    byte	*outPtr;
    int		row;

    if ( w == 1 && h == 1 )
        return;

    row = w * 4;
    outPtr = inPtr;
    w >>= 1;
    h >>= 1;

    if ( w == 0 || h == 0 ) {
        w += h;

        for ( y = 0; y < w; y++, outPtr += 4, inPtr += 8 ) {
            outPtr[0] = ( inPtr[0] + inPtr[4] ) >> 1;
            outPtr[1] = ( inPtr[1] + inPtr[5] ) >> 1;
            outPtr[2] = ( inPtr[2] + inPtr[6] ) >> 1;
            outPtr[3] = ( inPtr[3] + inPtr[7] ) >> 1;
        }
        return;
    }

    for ( y = 0; y < h; y++, inPtr += row ) {
        for ( k = 0; k < w; k++, outPtr += 4, inPtr += 8 ) {
            outPtr[0] = ( inPtr[0] + inPtr[4] + inPtr[row]     + inPtr[row + 4] ) >> 2;
            outPtr[1] = ( inPtr[1] + inPtr[5] + inPtr[row + 1] + inPtr[row + 5] ) >> 2;
            outPtr[2] = ( inPtr[2] + inPtr[6] + inPtr[row + 2] + inPtr[row + 6] ) >> 2;
            outPtr[3] = ( inPtr[3] + inPtr[7] + inPtr[row + 3] + inPtr[row + 7] ) >> 2;
        }
    }
}

/*
===================
resampleTexture
===================
*/
void R_Image::resampleTexture( unsigned *in, int inWidth, int inHeight, unsigned *out, int outWidth, int outHeight ) {
    int	y, k;
    unsigned *inRow, *inRow2;
    unsigned frac, fracStep;
    unsigned p1[Renderer::MaximumTextureSize], p2[Renderer::MaximumTextureSize];
    byte *pix1, *pix2, *pix3, *pix4;

    // failsafe
    if ( outWidth > Renderer::MaximumTextureSize )
        com.error( StrSoftError + this->tr( "could not resample texture (max width)\n" ));

    fracStep = inWidth * 0x10000 / outWidth;
    frac = fracStep >> 2;

    for ( y = 0; y < outWidth; y++ ) {
        p1[y] = 4 * ( frac >> 16 );
        frac += fracStep;
    }

    frac = 3 * ( fracStep >> 2 );
    for ( y = 0; y < outWidth; y++ ) {
        p2[y] = 4 * ( frac >> 16);
        frac += fracStep;
    }

    for ( y = 0; y < outHeight; y++, out += outWidth ) {
        inRow = in + inWidth * static_cast<int>(( y + 0.25f ) * inHeight / outHeight );
        inRow2 = in + inWidth * static_cast<int>(( y + 0.75f ) * inHeight / outHeight );

        frac = fracStep >> 1;

        for ( k = 0; k < outWidth; k++ ) {
            pix1 = reinterpret_cast<byte*>( inRow + p1[k] );
            pix2 = reinterpret_cast<byte*>( inRow + p2[k] );
            pix3 = reinterpret_cast<byte*>( inRow2 + p1[k] );
            pix4 = reinterpret_cast<byte*>( inRow2 + p2[k] );
            ( reinterpret_cast<byte*>( out + k ))[0] = ( pix1[0] + pix2[0] + pix3[0] + pix4[0] ) >> 2;
            ( reinterpret_cast<byte*>( out + k ))[1] = ( pix1[1] + pix2[1] + pix3[1] + pix4[1] ) >> 2;
            ( reinterpret_cast<byte*>( out + k ))[2] = ( pix1[2] + pix2[2] + pix3[2] + pix4[2] ) >> 2;
            ( reinterpret_cast<byte*>( out + k ))[3] = ( pix1[3] + pix2[3] + pix3[3] + pix4[3] ) >> 2;
        }
    }
}

/*
===================
createTexture
===================
*/
void R_Image::createTexture( byte *buffer, int w, int h ) {
    int scaledWidth, scaledHeight;
    byte *scaledBuffer, *resampledBuffer = NULL;
    int mipLevel = 0;

    // enable texturing
    glEnable( GL_TEXTURE_2D );

    // gen texture and get id
    glGenTextures( 1, &this->texture );

    // bind as current texure
    glBindTexture( GL_TEXTURE_2D, this->texture );

    // set wrap parms
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->clampModeGL());
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->clampModeGL());

    // convert to exact power of 2 sizes
    for ( scaledWidth = 1; scaledWidth < w; scaledWidth <<= 1 )
        ;
    for ( scaledHeight = 1; scaledHeight < h; scaledHeight <<= 1 )
        ;

    // resample image if required
    if ( scaledWidth != w || scaledHeight != h ) {
        resampledBuffer = new byte[scaledWidth * scaledHeight * 4];
        this->resampleTexture( reinterpret_cast<unsigned*>( buffer ), w, h, reinterpret_cast<unsigned*>( resampledBuffer ), scaledWidth, scaledHeight );
        buffer = resampledBuffer;
        w = scaledWidth;
        h = scaledHeight;
    }

    // clamp to the renderer texture upper size limit
    while ( scaledWidth > Renderer::MaximumTextureSize || scaledHeight > Renderer::MaximumTextureSize ) {
        scaledWidth >>= 1;
        scaledHeight >>= 1;
    }
    scaledBuffer = new byte[sizeof( unsigned ) * scaledWidth * scaledHeight];

    // copy or resample data as appropriate for first MIP level
    if (( scaledWidth == w ) && ( scaledHeight == h ))
        memcpy( scaledBuffer, buffer, w * h * 4 );
    else {
        // use the normal mip-mapping function to go down from here
        while ( w > scaledWidth || h > scaledHeight ) {
            this->mipMap( buffer, w, h );

            w >>= 1;
            h >>= 1;

            if ( w < 1 )
                w = 1;

            if ( h < 1 )
                h = 1;
        }
        memcpy( scaledBuffer, buffer, w * h * 4 );
    }

    this->setWidth( scaledWidth );
    this->setHeight( scaledHeight );

    // generate base texture
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, scaledWidth, scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaledBuffer );

    // generate mipmaps
    while ( scaledWidth > 1 || scaledHeight > 1 ) {
        this->mipMap( scaledBuffer, scaledWidth, scaledHeight );

        scaledWidth >>= 1;
        scaledHeight >>= 1;

        if ( scaledWidth < 1 )
            scaledWidth = 1;

        if ( scaledHeight < 1 )
            scaledHeight = 1;

        // advance by level
        mipLevel++;

        // generate texture for the mipmap
        glTexImage2D( GL_TEXTURE_2D, mipLevel, GL_RGBA, scaledWidth, scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaledBuffer );
    }

    // set filter
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

    // get rid of temporary buffers
    if ( resampledBuffer != NULL )
        delete [] resampledBuffer;
    delete [] scaledBuffer;

    // unbind texture
    glBindTexture( GL_TEXTURE_2D, 0 );

    // disable texturing
    glDisable( GL_TEXTURE_2D );
}

/*
===================
clampModeGL
===================
*/
GLint R_Image::clampModeGL() const {
    switch ( this->m_clamp ) {
    case Repeat:
        return GL_REPEAT;

    case Clamp:
        return GL_CLAMP;

    case ClampToEdge:
        return GL_CLAMP_TO_EDGE;

    default:
        com.error( StrSoftError + this->tr( "unknown clamp mode \"%1\"\n" ).arg( this->m_clamp ));
        return GL_REPEAT;
    }
}

/*
===================
loadTargaImage

 win32 is unable to load targas natively
===================
*/
#ifdef BUILTIN_TARGA_LOADER
byte *R_Image::loadTargaImage( const QString &filename, int len, const byte *buffer , unsigned int &rows, unsigned int &columns ) {
    unsigned     numPixels;
    byte         *pixelBuffer;
    int          row, column;
    TargaHeader  header;
    byte         *rgba;
    byte         *imageBuffer, *endBuffer;

    // check length
    if ( len < Renderer::TargaHeaderLength ) {
        com.error( StrSoftError + this->tr( "header of \"%1\" too short\n" ).arg( filename ));
        return NULL;
    }

    imageBuffer = ( byte* )buffer;
    endBuffer = imageBuffer + len;
    header.idLength = imageBuffer[0];
    header.colourmapType = imageBuffer[1];
    header.imageType = imageBuffer[2];

    memcpy( &header.colourmapIndex, &imageBuffer[3], 2 );
    memcpy( &header.colourmapLength, &imageBuffer[5], 2 );
    header.colormapSize = imageBuffer[7];
    memcpy( &header.xOrigin, &imageBuffer[8], 2 );
    memcpy( &header.yOrigin, &imageBuffer[10], 2 );
    memcpy( &header.width, &imageBuffer[12], 2 );
    memcpy( &header.height, &imageBuffer[14], 2 );
    header.pixelSize = imageBuffer[16];
    header.attributes = imageBuffer[17];
    header.colourmapIndex = qFromLittleEndian( header.colourmapIndex );
    header.colourmapLength = qFromLittleEndian( header.colourmapLength );
    header.xOrigin = qFromLittleEndian( header.xOrigin );
    header.yOrigin = qFromLittleEndian( header.yOrigin );
    header.width = qFromLittleEndian( header.width );
    header.height = qFromLittleEndian( header.height );
    imageBuffer += 18;

    if ( header.imageType != 2 && header.imageType != 10  && header.imageType != 3 )  {
        com.error( StrSoftError + this->tr( "(\"%1\") only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n" ).arg( filename ));
        return NULL;
    }

    if ( header.colourmapType != 0 ) {
        com.error( StrSoftError + this->tr( "(\"%1\") colormaps not supported\n" ).arg( filename ));
        return NULL;
    }

    if (( header.pixelSize != 32 && header.pixelSize != 24 ) && header.imageType != 3 ) {
        com.error( StrSoftError + this->tr( "(\"%1\") only 32 or 24 bit images supported\n" ).arg( filename ));
        return NULL;
    }

    columns = header.width;
    rows = header.height;
    numPixels = columns * rows * 4;

    if ( !columns || !rows || numPixels > 0x7FFFFFFF || numPixels / columns / 4 != rows ) {
        com.error( StrSoftError + this->tr( "invalid image size in \"%1\"\n" ).arg( filename ));
        return NULL;
    }
    rgba = new byte[numPixels];

    if ( header.idLength != 0 ) {
        if ( imageBuffer + header.idLength > endBuffer ) {
            com.error( StrSoftError + this->tr( "header in \"%1\" too short\n" ).arg( filename ));
            return NULL;
        }

        // skip TARGA image comment
        imageBuffer += header.idLength;
    }

    if ( header.imageType == 2 || header.imageType == 3 ) {
        if ( imageBuffer + columns * rows * header.pixelSize / 8 > endBuffer ) {
            com.error( StrSoftError + this->tr( "file \"%1\" truncated\n" ).arg( filename ));
            return NULL;
        }

        // uncompressed RGB or gray scale image
        for ( row = rows - 1; row >= 0; row-- ) {
            pixelBuffer = rgba + row * columns * 4;

            for ( column = 0; column < static_cast<int>( columns ); column++ ) {
                unsigned char red, green, blue, alphabyte;

                switch ( header.pixelSize ) {
                case 8:
                    blue = *imageBuffer++;
                    green = blue;
                    red = blue;
                    *pixelBuffer++ = red;
                    *pixelBuffer++ = green;
                    *pixelBuffer++ = blue;
                    *pixelBuffer++ = 255;
                    break;

                case 24:
                    blue = *imageBuffer++;
                    green = *imageBuffer++;
                    red = *imageBuffer++;
                    *pixelBuffer++ = red;
                    *pixelBuffer++ = green;
                    *pixelBuffer++ = blue;
                    *pixelBuffer++ = 255;
                    break;

                case 32:
                    blue = *imageBuffer++;
                    green = *imageBuffer++;
                    red = *imageBuffer++;
                    alphabyte = *imageBuffer++;
                    *pixelBuffer++ = red;
                    *pixelBuffer++ = green;
                    *pixelBuffer++ = blue;
                    *pixelBuffer++ = alphabyte;
                    break;

                default:
                    com.error( StrSoftError + this->tr( "illegal pixelSize '%1' in \"%2\"\n" ).arg( header.pixelSize ).arg( filename ));
                    return NULL;
                }

            }
        }
        // runlen encoded RGB images
    } else if ( header.imageType == 10 ) {
        unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;

        red = 0;
        green = 0;
        blue = 0;
        alphabyte = 0xff;

        for ( row = rows - 1; row >= 0; row-- ) {
            pixelBuffer = rgba + row * columns * 4;

            for ( column = 0; column < static_cast<int>( columns ); ) {
                if ( imageBuffer + 1 > endBuffer ) {
                    com.error( StrSoftError + this->tr( "file \"%1\" truncated\n" ).arg( filename ));
                    return NULL;
                }
                packetHeader = *imageBuffer++;
                packetSize = 1 + ( packetHeader & 0x7f );

                // run-len packet
                if ( packetHeader & 0x80 ) {
                    if ( imageBuffer + header.pixelSize / 8 > endBuffer ) {
                        com.error( StrSoftError + this->tr( "file \"%1\" truncated\n" ).arg( filename ));
                        return NULL;
                    }
                    switch ( header.pixelSize ) {
                    case 24:
                        blue = *imageBuffer++;
                        green = *imageBuffer++;
                        red = *imageBuffer++;
                        alphabyte = 255;
                        break;

                    case 32:
                        blue = *imageBuffer++;
                        green = *imageBuffer++;
                        red = *imageBuffer++;
                        alphabyte = *imageBuffer++;
                        break;

                    default:
                        com.error( StrSoftError + this->tr( "illegal pixelSize '%1' in \"%2\"\n" ).arg( header.pixelSize ).arg( filename ));
                        return NULL;
                    }

                    for ( j = 0; j < packetSize; j++ ) {
                        *pixelBuffer++ = red;
                        *pixelBuffer++ = green;
                        *pixelBuffer++ = blue;
                        *pixelBuffer++ = alphabyte;
                        column++;

                        // run spans across rows
                        if ( column == static_cast<int>( columns )) {
                            column = 0;

                            if ( row > 0 )
                                row--;
                            else
                                goto breakOut;
                            pixelBuffer = rgba + row * columns * 4;
                        }
                    }
                    // non run-len packet
                } else {
                    if ( imageBuffer + header.pixelSize / 8 * packetSize > endBuffer ) {
                        com.error( StrSoftError + this->tr( "file \"%1\" truncated\n" ).arg( filename ));
                        return NULL;
                    }

                    for ( j = 0; j < packetSize; j++ ) {
                        switch ( header.pixelSize ) {
                        case 24:
                            blue = *imageBuffer++;
                            green = *imageBuffer++;
                            red = *imageBuffer++;
                            *pixelBuffer++ = red;
                            *pixelBuffer++ = green;
                            *pixelBuffer++ = blue;
                            *pixelBuffer++ = 255;
                            break;

                        case 32:
                            blue = *imageBuffer++;
                            green = *imageBuffer++;
                            red = *imageBuffer++;
                            alphabyte = *imageBuffer++;
                            *pixelBuffer++ = red;
                            *pixelBuffer++ = green;
                            *pixelBuffer++ = blue;
                            *pixelBuffer++ = alphabyte;
                            break;

                        default:
                            com.error( StrSoftError + this->tr( "illegal pixelSize '%1' in \"%2\"\n" ).arg( header.pixelSize ).arg( filename ));
                            return NULL;
                        }
                        column++;

                        // pixel packet run spans across rows
                        if ( column == static_cast<int>( columns )) {
                            column = 0;

                            if ( row > 0 )
                                row--;
                            else
                                goto breakOut;

                            pixelBuffer = rgba + row * columns * 4;
                        }
                    }
                }
            }
breakOut:;
        }
    }

    // print a warning
    if ( header.attributes & 0x20 )
        com.print( StrWarn + this->tr( "TGA file header in \"%1\" declares top-down image, ignoring\n" ).arg( filename ));

    // return
    return rgba;
}
#endif
