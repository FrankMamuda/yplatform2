/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

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
#include "nf_main.h"
#include "nf_devlist.h"
#include "nf_devenum.h"
#include "nf_storagedrive.h"
#include "nf_scanner.h"
#include "../mod_trap.h"
#include "../../common/sys_common.h"
#include <taglib/tag.h>
#include <taglib/fileref.h>

/*
================
run
================
*/
void Scanner::run() {
    // let the drive settle
    this->sleep( 2 );

    // scan the volume
    this->scan();


    // we're done
    quit();
}

/*
================
filter
================
*/
void Scanner::filter( const QString &text, const QString &filePath ) {
    foreach ( QString str, this->m_blockables ) {
        if ( text.contains( str, Qt::CaseInsensitive )) {
            emit panic( filePath );
            return;
        }
    }
}

/*
================
scan
================
*/
void Scanner::scan() {
    QDirIterator dir( this->path(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories );
    while ( dir.hasNext()) {
        dir.next();

        if ( dir.fileInfo().completeSuffix() == "mp3" ) {
            // first pass - filenames
            this->filter( dir.fileName(), dir.filePath());

            // second pass - tags
            if ( !this->checkTags( dir.filePath()))
                emit panic( dir.filePath());
        }
    }
}

/*
================
checkTags
================
*/
bool Scanner::checkTags( const QString &filePath ) {
    TagLib::FileRef f( filePath.toLatin1().constData());
    TagLib::String artist = f.tag()->artist();
    TagLib::String title = f.tag()->title();
    TagLib::String genre = f.tag()->genre();

    foreach ( QString str, this->m_blockables ) {
        if ( QString( artist.toCString()).contains( str, Qt::CaseInsensitive ) ||
             QString( title.toCString()).contains( str, Qt::CaseInsensitive ) ||
             QString( genre.toCString()).contains( str, Qt::CaseInsensitive ))

            return false;

    }

    return true;
}

