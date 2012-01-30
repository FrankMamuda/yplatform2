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

#ifndef R_IMAGE_H
#define R_IMAGE_H

//
// includes
//
#include "r_shared.h"

//
// namespaces
//
#ifdef Q_OS_WIN
namespace Renderer {
    static const int TargaHeaderLength = 18;
}
#define BUILTIN_TARGA_LOADER
#endif

//
// class:R_Image
//
class R_Image : public QObject {
    Q_OBJECT
    Q_CLASSINFO( "description", "Image loader" )
    Q_DISABLE_COPY( R_Image )
    Q_PROPERTY( bool valid READ isValid WRITE validate )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( ClampModes clampMode READ clampMode WRITE setClampMode )
    Q_PROPERTY( int width READ width WRITE setWidth )
    Q_PROPERTY( int height READ height WRITE setHeight )
    Q_ENUMS( ClampModes )

public:
    enum ClampModes {
        Repeat = 0,
        Clamp,
        ClampToEdge
    };

    explicit R_Image( const QString &, ClampModes mode = Repeat, QObject * = 0 );
    void reload( const QString &filename, ClampModes mode = Repeat );
    ~R_Image();
    GLuint texture;
    int width() const { return this->m_width; }
    int height() const { return this->m_height; }

    // property getters
    bool isValid() const { return this->m_valid; }
    QString name() const { return this->m_name; }
    ClampModes clampMode() const { return this->m_clamp; }
    GLint clampModeGL() const;

private:
#ifdef BUILTIN_TARGA_LOADER
    byte *loadTargaImage( const QString &, int, const byte *, unsigned int &, unsigned int & );
#endif
    void createTexture( const byte *buffer, int w, int h );

    // properties
    bool m_valid;
    QString m_name;
    ClampModes m_clamp;
    int m_width;
    int m_height;

    // targa related
#ifdef BUILTIN_TARGA_LOADER
    struct TargaHeader {
        unsigned char   idLength, colourmapType, imageType;
        unsigned short  colourmapIndex, colourmapLength;
        unsigned char   colormapSize;
        unsigned short  xOrigin, yOrigin, width, height;
        unsigned char   pixelSize, attributes;
    };
#endif

private slots:
    void setWidth( int w ) { this->m_width = w; }
    void setHeight( int h ) { this->m_height = h; }

public slots:
    // property setters
    void validate( bool valid = true ) { this->m_valid = valid; }
    void setName( const QString &name ) { this->m_name = name; }
    void setClampMode( ClampModes mode ) { this->m_clamp = mode; }
};

#endif // R_IMAGE_H
