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

#ifndef SYS_LINK_H
#define SYS_LINK_H

//
// includes
//
#include "sys_shared.h"

// failsafe
#ifndef Q_OS_WIN
#error "Links work only on Windows"
#endif

//
// win32 link handling
//
typedef struct lnkInfo_s {
    lnkInfo_s() : fileSize(0), isNetworkPath( false ), isDirectory( false ), isFileOrDir( false ) {
    }
    quint32 fileSize;
    bool isNetworkPath;
    bool isDirectory;
    bool isFileOrDir;
    QString volumeName;
    QString driveName;
    QString path;
    QString description;
} lnkInfo_t;

typedef struct lnkHeader_s {
    char magic[4];
    char GUID[16];
    quint32 flags;
    quint32 attributes;
    char time1[8];
    char time2[8];
    char time3[8];
    quint32 length;
    quint32 iconNum;
    quint32 showWnd;
    quint32 hotKey;
    char filler[8];
} lnkHeader_t;

typedef struct lnkFileLocation_s {
    quint32 totalLen;
    quint32 ptr;
    quint32 flags;
    quint32 localVolume;
    quint32 basePath;
    quint32 netVolume;
    quint32 pathname;
} lnkFileLocation_t;

#endif // SYS_LINK_H
