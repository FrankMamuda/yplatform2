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

#ifndef R_TEXTURE_H
#define R_TEXTURE_H

//
// includes
//
#include <QOpenGLTexture>

//
// class: R_Texture
//
class R_Texture : public QOpenGLTexture {
    Q_CLASSINFO( "description", "Texture loader" )
    Q_PROPERTY( QString filename READ filename WRITE setFilename )

public:
    R_Texture( const QString &filename, WrapMode mode = Repeat );
    QString filename() const { return this->m_filename; }

signals:

public slots:
    void setFilename( const QString &filename ) { this->m_filename = filename; }

private:
    Q_DISABLE_COPY( R_Texture )
    QString m_filename;
};

#endif // R_TEXTURE_H
