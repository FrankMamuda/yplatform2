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
#include "r_main.h"
#include "../modules/mod_trap.h"
#include "../common/sys_common.h"

/*
===================
construct
===================
*/
R_Image::R_Image( const QString &filename, ClampModes mode, QObject *parent ) : QObject( parent ) {
    QImage image;
    byte *buffer;
    byte *imageBuffer = NULL;
    int len;
#ifdef BUILTIN_TARGA_LOADER
    bool targa = false;
#endif

    // defaults
    this->validate( false );

    // set clamp mode
    this->setClampMode( mode );

    // load image
    len = mt.fsReadFile( filename, &buffer, Sys_Filesystem::Silent );
    if ( len > 0 ) {
#ifdef BUILTIN_TARGA_LOADER
        if ( filename.endsWith( ".tga" )) {
            // win32 can't handle targas natively
            unsigned int w, h;
            imageBuffer = this->loadTargaImage( filename, len, buffer, w, h );
            this->setWidth( w );
            this->setHeight( h );
            targa = true;
        } else {
#endif
            image = QGLWidget::convertToGLFormat( QImage::fromData( buffer, len ).mirrored( false, true ));
            imageBuffer = image.bits();
            this->setWidth( image.width());
            this->setHeight( image.height());
#ifdef BUILTIN_TARGA_LOADER
        }
#endif

        // free buffer
        mt.fsFreeFile( filename );
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
destruct
===================
*/
R_Image::~R_Image() {
    glDeleteTextures( 1, &this->texture );
}

/*
===================
createTexture
===================
*/
void R_Image::createTexture( const byte *buffer, int w, int h ) {
    // enable texturing
    glEnable( GL_TEXTURE_2D );

    // gen texture and get id
    glGenTextures( 1, &this->texture );

    // bind as current texure
    glBindTexture( GL_TEXTURE_2D, this->texture );

    // set wrap parms
    // GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->clampModeGL());
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->clampModeGL());

    // build mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer );

    // upload texture data
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );

    // set filter
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

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
        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::clampMode: unknown clamp mode \"%1\"\n" ).arg( this->m_clamp ));
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
        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") header too short\n" ).arg( filename ));
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
    header.colourmapIndex = littleShort( header.colourmapIndex );
    header.colourmapLength = littleShort( header.colourmapLength );
    header.xOrigin = littleShort( header.xOrigin );
    header.yOrigin = littleShort( header.yOrigin );
    header.width = littleShort( header.width );
    header.height = littleShort( header.height );
    imageBuffer += 18;

    if ( header.imageType != 2 && header.imageType != 10  && header.imageType != 3 )  {
        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n" ).arg( filename ));
        return NULL;
    }

    if ( header.colourmapType != 0 ) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") colormaps not supported\n" ).arg( filename ));
        return NULL;
    }

    if (( header.pixelSize != 32 && header.pixelSize != 24 ) && header.imageType != 3 ) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") only 32 or 24 bit images supported\n" ).arg( filename ));
        return NULL;
    }

    columns = header.width;
    rows = header.height;
    numPixels = columns * rows * 4;

    if ( !columns || !rows || numPixels > 0x7FFFFFFF || numPixels / columns / 4 != rows ) {
        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") invalid image size\n" ).arg( filename ));
        return NULL;
    }
    rgba = new byte[numPixels];

    if ( header.idLength != 0 ) {
        if ( imageBuffer + header.idLength > endBuffer ) {
            mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") header too short\n" ).arg( filename ));
            return NULL;
        }

        // skip TARGA image comment
        imageBuffer += header.idLength;
    }

    if ( header.imageType == 2 || header.imageType == 3 ) {
        if ( imageBuffer + columns * rows * header.pixelSize / 8 > endBuffer ) {
            mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") file truncated\n" ).arg( filename ));
            return NULL;
        }

        // uncompressed RGB or gray scale image
        for ( row = rows - 1; row >= 0; row-- ) {
            pixelBuffer = rgba + row * columns * 4;

            for ( column = 0; column < (int)columns; column++ ) {
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
                    mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") illegal pixelSize '%2'\n" ).arg( filename ).arg( header.pixelSize ));
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

            for ( column = 0; column < (int)columns; ) {
                if ( imageBuffer + 1 > endBuffer ) {
                    mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") file truncated\n" ).arg( filename ));
                    return NULL;
                }
                packetHeader = *imageBuffer++;
                packetSize = 1 + ( packetHeader & 0x7f );

                // run-len packet
                if ( packetHeader & 0x80 ) {
                    if ( imageBuffer + header.pixelSize / 8 > endBuffer ) {
                        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") file truncated\n" ).arg( filename ));
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
                        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") illegal pixelSize '%2'\n" ).arg( filename ).arg( header.pixelSize ));
                        return NULL;
                    }

                    for ( j = 0; j < packetSize; j++ ) {
                        *pixelBuffer++ = red;
                        *pixelBuffer++ = green;
                        *pixelBuffer++ = blue;
                        *pixelBuffer++ = alphabyte;
                        column++;

                        // run spans across rows
                        if ( column == (int)columns ) {
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
                        mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") file truncated\n" ).arg( filename ));
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
                            mt.comError( Sys_Common::SoftError, this->tr( "R_Image::loadTargaImage: (\"%1\") illegal pixelSize '%2'\n" ).arg( filename ).arg( header.pixelSize ));
                            return NULL;
                        }
                        column++;

                        // pixel packet run spans across rows
                        if ( column == (int)columns ) {
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
        mt.comPrint( this->tr( "^3WARNING: R_Image::loadTargaImage: (\"%1\") TGA file header declares top-down image, ignoring\n" ).arg( filename ));

    // return
    return rgba;
}
#endif
