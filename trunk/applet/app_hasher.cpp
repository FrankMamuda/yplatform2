/*
===========================================================================
Copyright (C) 2009-2012 Edd 'Double Dee' Psycho

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
// simple cryptographic encryption for cvars, etc.
//

//
// includes
//
#include "app_hasher.h"
#include <QCryptographicHash>
#include <QString>

//
// classes
//
App_Hasher hash;

/*
================
hashForString
================
*/
QString App_Hasher::encrypt( const QString &input ) {
    QCryptographicHash *hash = new QCryptographicHash( QCryptographicHash::Md5 );
    hash->addData( input.toLatin1().constData(), input.length());
    return QString( hash->result().toHex().constData());
}
