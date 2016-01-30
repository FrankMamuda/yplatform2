/*
===========================================================================
Copyright (C) 2011-2016 Edd 'Double Dee' Psycho

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
// include
//
#include "r_texture.h"
#include "r_main.h"

/*
===================
construct
===================
*/
R_Texture::R_Texture( const QString &filename, WrapMode mode ) : QOpenGLTexture( QImage( filename ) /*QOpenGLTexture::Target2D*/ ) {
    this->destroy();

    QByteArray buffer = fs.readFile( filename, Sys_Filesystem::Silent );
    if ( !buffer.isEmpty()) {
        this->setData( QImage::fromData( buffer ));
        this->setMinificationFilter( QOpenGLTexture::LinearMipMapLinear );
        this->setMagnificationFilter( QOpenGLTexture::Linear );
    }

    buffer.clear();
    this->setWrapMode( mode );
}
