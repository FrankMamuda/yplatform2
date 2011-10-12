/*
===========================================================================
Copyright (C) 2009-2011 Edd 'Double Dee' Psycho

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
// YPlatform 2 Shared Header
//

#ifndef SYSSHARED_H
#define SYSSHARED_H

//
// includes
//
#include <QtGlobal>
#include <QtCore>

//
// defines
//
namespace Sys {
    static const QString Title       ( QObject::trUtf8( "YPlatform2" ));
    static const QString Copyright   ( QObject::trUtf8( "Copyright (c) 2009-2011, Edd 'Double Dee' Psycho." ));
    static const QString Version     ( QObject::trUtf8( "Yuna Commons: v2.8.99.5 (rc6)" ));
    static const QString GUIVersion  ( QObject::trUtf8( "GUI: v1.0.1" ));

    // colours
    static const QChar ColourEscape  ( '^' );
    static const QChar ColourBlack   ( '0' );
    static const QChar ColourRed     ( '1' );
    static const QChar ColourGreen   ( '2' );
    static const QChar ColourYellow  ( '3' );
    static const QChar ColourBlue    ( '4' );
    static const QChar ColourCyan    ( '5' );
    static const QChar ColourMagenta ( '6' );
    static const QChar ColourWhite   ( '7' );
    static const QChar ColourNull    ( '*' );
}

// byte
typedef unsigned char byte;

// fs file
typedef int fileHandle_t;

//
// platform-specific
//
#ifdef Q_OS_WIN
#  define LIBRARY_PREFIX ""
#  define LIBRARY_EXT "dll"
#  define LIBRARY_SUFFIX "win"
#  define YP_LITTLE_ENDIAN
#else
#  define LIBRARY_PREFIX "lib"
#endif

// windows
#ifdef Q_OS_WIN
#  ifdef Q_OS_WIN32
#    define ARCH_STRING "x86"
#  elif defined ( Q_OS_WIN64 )
#    define ARCH_STRING "x86_64"
#  endif
#endif

// darwin
#if defined( Q_OS_DARWIN )
#  if defined( Q_OS_DARWIN64 )
#    define ARCH_STRING "x86_64"
#  elif defined( Q_OS_DARWIN32 )
#    define ARCH_STRING "i386"
#  endif
#  define LIBRARY_EXT "dylib"
#  define LIBRARY_SUFFIX "darwin"
#  define YP_LITTLE_ENDIAN
#endif

#ifdef Q_OS_LINUX
#  if defined __i386__
#    define ARCH_STRING "i386"
#  elif defined __x86_64__
#    define ARCH_STRING "x86_64"
#  elif defined __powerpc64__
#    define ARCH_STRING "ppc64"
#  elif defined __powerpc__
#    define ARCH_STRING "ppc"
#  elif defined __s390__
#    define ARCH_STRING "s390"
#  elif defined __s390x__
#    define ARCH_STRING "s390x"
#  elif defined __ia64__
#    define ARCH_STRING "ia64"
#  elif defined __alpha__
#    define ARCH_STRING "alpha"
#  elif defined __sparc__
#    define ARCH_STRING "sparc"
#  elif defined __arm__
#    define ARCH_STRING "arm"
#  elif defined __cris__
#    define ARCH_STRING "cris"
#  elif defined __hppa__
#    define ARCH_STRING "hppa"
#  elif defined __mips__
#    define ARCH_STRING "mips"
#  elif defined __sh__
#    define ARCH_STRING "sh"
#  endif
#  define LIBRARY_EXT "so"
#  define LIBRARY_SUFFIX "linux"
#  include <endian.h>
#  if __FLOAT_WORD_ORDER == __BIG_ENDIAN
#    define YP_BIG_ENDIAN
#  else
#    define YP_LITTLE_ENDIAN
#  endif
#endif

#ifdef Q_OS_FREEBSD
#  ifdef __i386__
#    define ARCH_STRING "i386"
#  elif defined __amd64__
#    define ARCH_STRING "amd64"
#  elif defined __axp__
#    define ARCH_STRING "alpha"
#  endif
#  define LIBRARY_EXT "so"
#  define LIBRARY_SUFFIX "freebsd"
#  include <machine/endian.h>
#  if BYTE_ORDER == BIG_ENDIAN
#    define YP_BIG_ENDIAN
#  else
#    define YP_LITTLE_ENDIAN
#  endif
#endif

#ifdef Q_OS_SOLARIS
#  ifdef __i386__
#    define ARCH_STRING "i386"
#  elif defined __sparc
#    define ARCH_STRING "sparc"
#  endif
#  define LIBRARY_EXT "so"
#  define LIBRARY_SUFFIX "solaris"
#  include <sys/isa_defs.h>
#  include <sys/byteorder.h>
#  if defined ( _BIG_ENDIAN )
#    define YP_BIG_ENDIAN
#  elif defined ( _LITTLE_ENDIAN )
#    define YP_LITTLE_ENDIAN
#  endif
#endif

#ifndef ARCH_STRING
#  error "arch not supported"
#endif

#if defined ( YP_BIG_ENDIAN ) && defined ( YP_LITTLE_ENDIAN )
#  error "endianness defined as both big and little"
#elif defined ( YP_BIG_ENDIAN )
#  define littleShort( x ) shortSwap( x )
#  define littleLong( x ) longSwap( x )
#  define littleFloat( x ) floatSwap( &x )
#  define bigShort
#  define bigLong
#  define bigFloat
#elif defined ( YP_LITTLE_ENDIAN )
#  define littleShort
#  define littleLong
#  define littleFloat
#  define bigShort( x ) shortSwap( x )
#  define bigLong( x ) longSwap( x )
#  define bigFloat( x ) floatSwap( &x )
#else
#  error "endianness not defined"
#  endif
#endif
